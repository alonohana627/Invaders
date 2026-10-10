#include "session.hpp"

#include <gamelib/core/collision.hpp>

#include <algorithm>
#include <cmath>

namespace invaders::game {

using gamelib::core::Cell;
using gamelib::core::Rect;
using gamelib::core::Vec2;

namespace {

[[nodiscard]] Rect _bombRect(const Rules& rules, const Bomb& bomb)
{
    return Rect::fromCenter(bomb.position.value(), {rules.bombWidth, rules.bombHeight});
}

} // namespace

Session::Session(const std::uint64_t seed, const float tickSeconds, Rules rules)
    : m_rules{rules}
    , m_dt{tickSeconds}
    , m_rng{seed}
    , m_cannonX{(m_rules.left + m_rules.right) * 0.5f}
    , m_lives{m_rules.lives}
{
    m_saucerTimer.start(ticks(m_rng.uniform(m_rules.saucerIntervalMin, m_rules.saucerIntervalMax)));
    startWave();
}

float Session::bombSpeed() const
{
    const float bonus = std::min(m_rules.bombSpeedMaxBonus, m_rules.bombSpeedPerWave * static_cast<float>(m_wave - 1));
    return m_rules.bombSpeed * (1.0f + bonus);
}

int Session::stepTicks() const
{
    const float seconds = m_rules.stepBaseSeconds + m_rules.stepPerAlienSeconds * static_cast<float>(m_formation.aliveCount());
    return std::max(1, ticks(seconds));
}

void Session::startWave()
{
    ++m_wave;
    // Each wave starts a row lower than the last, for a few waves; then the
    // depth starts over (the arcade does the same).
    const int depth = (m_wave - 1) % (m_rules.maxWaveDrops + 1);
    m_formation = Formation{m_rules, m_rules.firstRowY + m_rules.waveDrop * static_cast<float>(depth)};
    const float spacing = (m_rules.right - m_rules.left) / static_cast<float>(Rules::kShields);
    const float width = m_rules.shieldCell * Shield::kWidth;
    for (int i = 0; i < Rules::kShields; ++i) {
        const float center = m_rules.left + spacing * (static_cast<float>(i) + 0.5f);
        m_shields[static_cast<std::size_t>(i)] = Shield{{center - width * 0.5f, m_rules.shieldTop}, m_rules.shieldCell};
    }
    m_bombs.clear();
    m_shot.reset();
    m_sinceStep = 0;
    m_note = 0;
    m_bombTimer.start(ticks(1.6f)); // a moment of calm before the first bomb
    m_phase = Phase::Playing;
    m_events.push({.kind = EventKind::WaveStarted, .value = m_wave});
}

void Session::tick(const Input& input)
{
    m_cannonX.beginTick();
    if (m_shot) {
        m_shot->beginTick();
    }
    for (Bomb& bomb : m_bombs) {
        bomb.position.beginTick();
    }
    if (m_saucer) {
        m_saucer->position.beginTick();
    }

    switch (m_phase) {
    case Phase::Playing:
        moveCannon(input);
        moveShot();
        moveSaucer();
        moveBombs();
        if (m_phase == Phase::Playing) {
            march();
        }
        if (m_phase == Phase::Playing) {
            dropBombs();
        }
        break;
    case Phase::WaveClear:
        moveCannon(input);
        moveShot();
        if (m_phaseTimer.tick()) {
            startWave();
        }
        break;
    case Phase::Dying:
        if (m_phaseTimer.tick()) {
            finishPhase();
        }
        break;
    case Phase::GameOver: break;
    }
}

void Session::moveCannon(const Input& input)
{
    const float half = m_rules.cannonWidth * 0.5f;
    const float move = std::clamp(input.move, -1.0f, 1.0f) * m_rules.cannonSpeed * m_dt;
    m_cannonX.value() = std::clamp(m_cannonX.value() + move, m_rules.left + half, m_rules.right - half);

    if (input.fire && !m_shot) {
        const Vec2 start{m_cannonX.value(), m_rules.cannonY - m_rules.cannonHeight * 0.5f - m_rules.shotHeight * 0.5f};
        m_shot.emplace(start);
        ++m_shotsFired;
        m_events.push({.kind = EventKind::Fired, .position = start});
    }
}

void Session::moveShot()
{
    if (!m_shot) {
        return;
    }
    // Several sub-steps per tick, each shorter than a shield cell, so the shot
    // never jumps over a cell, a bomb or the edge of an alien.
    const float distance = m_rules.shotSpeed * m_dt;
    const int steps = std::max(1, static_cast<int>(std::ceil(distance / (m_rules.shieldCell * 0.75f))));
    for (int i = 0; i < steps; ++i) {
        Vec2& at = m_shot->value();
        at.y -= distance / static_cast<float>(steps);
        if (shotHits(at)) {
            m_shot.reset();
            return;
        }
        if (at.y + m_rules.shotHeight * 0.5f < m_rules.top) {
            m_events.push({.kind = EventKind::ShotMissed, .position = {at.x, m_rules.top}});
            m_shot.reset();
            return;
        }
    }
}

bool Session::shotHits(const Vec2 at)
{
    const Rect shot = Rect::fromCenter(at, {m_rules.shotWidth, m_rules.shotHeight});
    const Vec2 tip{at.x, shot.top()};

    for (auto bomb = m_bombs.begin(); bomb != m_bombs.end(); ++bomb) {
        if (gamelib::core::overlaps(shot, _bombRect(m_rules, *bomb))) {
            m_events.push({.kind = EventKind::ShotHitBomb, .position = (tip + bomb->position.value()) * 0.5f});
            m_bombs.erase(bomb);
            return true;
        }
    }

    if (m_saucer
        && gamelib::core::overlaps(shot, Rect::fromCenter(m_saucer->position.value(), {m_rules.saucerWidth, m_rules.saucerHeight}))) {
        const int points = m_rules.saucerPoints[static_cast<std::size_t>(m_shotsFired % 15)];
        m_events.push({.kind = EventKind::SaucerKilled, .position = m_saucer->position.value(), .points = points});
        m_saucer.reset();
        m_saucerTimer.start(ticks(m_rng.uniform(m_rules.saucerIntervalMin, m_rules.saucerIntervalMax)));
        addScore(points);
        return true;
    }

    for (int row = Rules::kRows - 1; row >= 0; --row) {
        for (int column = 0; column < Rules::kColumns; ++column) {
            const Cell cell{column, row};
            if (!m_formation.alive(cell) || !gamelib::core::overlaps(shot, m_formation.rect(cell))) {
                continue;
            }
            const AlienType type = Formation::type(row);
            const int points = m_rules.alienPoints[static_cast<std::size_t>(type)];
            m_events.push({.kind = EventKind::AlienKilled, .position = m_formation.center(cell), .points = points, .alien = type});
            m_formation.kill(cell);
            addScore(points);
            if (m_formation.aliveCount() == 0) {
                m_phase = Phase::WaveClear;
                m_phaseTimer.start(ticks(m_rules.waveClearSeconds));
                m_bombs.clear();
                if (m_saucer) {
                    m_saucer.reset();
                    m_events.push({.kind = EventKind::SaucerLeft});
                }
                m_events.push({.kind = EventKind::WaveCleared});
            }
            return true;
        }
    }

    if (Shield* shield = shieldAt(tip); shield != nullptr && shield->solidAt(tip)) {
        shield->blast(tip, m_rules.shotBlast, m_rules.shotCrumble, m_rng);
        m_events.push({.kind = EventKind::ShieldHit, .position = tip, .value = 0});
        return true;
    }
    return false;
}

void Session::moveBombs()
{
    const float distance = bombSpeed() * m_dt;
    for (std::size_t i = 0; i < m_bombs.size();) {
        Bomb& bomb = m_bombs[i];
        bomb.position.value().y += distance;
        const Rect box = _bombRect(m_rules, bomb);
        const Vec2 tip{box.center().x, box.bottom()};

        if (gamelib::core::overlaps(box, cannonRect())) {
            killCannon(); // clears every bomb
            return;
        }
        Shield* shield = shieldAt(tip);
        const float side = m_rules.bombWidth * 0.5f - 1.0f;
        if (shield != nullptr
            && (shield->solidAt(tip) || shield->solidAt(tip + Vec2{-side, 0.0f}) || shield->solidAt(tip + Vec2{side, 0.0f}))) {
            shield->blast(tip + Vec2{0.0f, m_rules.shieldCell * 0.5f}, m_rules.bombBlast, m_rules.bombCrumble, m_rng);
            m_events.push({.kind = EventKind::ShieldHit, .position = tip, .value = 1, .bomb = bomb.type});
            m_bombs.erase(m_bombs.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }
        if (tip.y >= m_rules.ground) {
            m_events.push({.kind = EventKind::BombLanded, .position = {tip.x, m_rules.ground}, .bomb = bomb.type});
            m_bombs.erase(m_bombs.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }
        ++i;
    }
}

void Session::moveSaucer()
{
    const float half = m_rules.saucerWidth * 0.5f;
    if (m_saucer) {
        Vec2& at = m_saucer->position.value();
        at.x += m_saucer->direction * m_rules.saucerSpeed * m_dt;
        if (at.x > m_rules.right - half || at.x < m_rules.left + half) {
            m_saucer.reset();
            m_events.push({.kind = EventKind::SaucerLeft});
            m_saucerTimer.start(ticks(m_rng.uniform(m_rules.saucerIntervalMin, m_rules.saucerIntervalMax)));
        }
        return;
    }
    if (!m_saucerTimer.tick()) {
        return;
    }
    if (m_formation.aliveCount() < m_rules.saucerMinAliens) {
        m_saucerTimer.start(ticks(m_rules.saucerIntervalMin));
        return;
    }
    // Like the arcade, the side depends on the shots fired: an even count
    // sends it from the left.
    const float direction = m_shotsFired % 2 == 0 ? 1.0f : -1.0f;
    const Vec2 start{direction > 0.0f ? m_rules.left + half : m_rules.right - half, m_rules.saucerY};
    m_saucer.emplace(Saucer{.position = gamelib::core::Interpolated<Vec2>{start}, .direction = direction});
    m_events.push({.kind = EventKind::SaucerAppeared, .position = start});
}

void Session::march()
{
    if (++m_sinceStep < stepTicks()) {
        return;
    }
    m_sinceStep = 0;
    m_formation.step();
    m_events.push({.kind = EventKind::Step, .value = m_note});
    m_note = (m_note + 1) % 4;

    // Aliens marching through a shield eat it away.
    for (int row = 0; row < Rules::kRows; ++row) {
        for (int column = 0; column < Rules::kColumns; ++column) {
            if (!m_formation.alive({column, row})) {
                continue;
            }
            const Rect alien = m_formation.rect({column, row});
            for (Shield& shield : m_shields) {
                if (gamelib::core::overlaps(alien, shield.bounds())) {
                    shield.erase(alien);
                }
            }
        }
    }

    if (m_formation.aliveCount() > 0 && m_formation.bounds().bottom() >= m_rules.invasionLine()) {
        m_events.push({.kind = EventKind::Invaded, .position = {m_cannonX.value(), m_rules.cannonY}});
        m_lives = 1; // killCannon takes the last one: the game ends after the explosion
        killCannon();
    }
}

void Session::dropBombs()
{
    if (!m_bombTimer.tick()) {
        return;
    }
    const float factor = std::max(m_rules.bombIntervalFloor,
                                  std::pow(m_rules.bombIntervalPerWave, static_cast<float>(m_wave - 1)));
    m_bombTimer.start(ticks(m_rng.uniform(m_rules.bombIntervalMin, m_rules.bombIntervalMax) * factor));

    // Each kind of bomb may be on screen once; the kinds take turns.
    int type = -1;
    for (int k = 0; k < kBombTypes; ++k) {
        const int candidate = (m_nextBomb + k) % kBombTypes;
        if (std::ranges::none_of(m_bombs, [&](const Bomb& b) { return static_cast<int>(b.type) == candidate; })) {
            type = candidate;
            break;
        }
    }
    if (type < 0) {
        return;
    }

    std::vector<int> columns;
    for (int column = 0; column < Rules::kColumns; ++column) {
        if (m_formation.lowest(column)) {
            columns.push_back(column);
        }
    }
    if (columns.empty()) {
        return;
    }
    int column = columns[m_rng.below(static_cast<std::uint32_t>(columns.size()))];
    if (static_cast<BombType>(type) == BombType::Rolling) {
        // The rolling bomb is aimed: it falls from the column nearest the cannon.
        const float x = m_cannonX.value();
        column = *std::ranges::min_element(columns, {}, [&](const int c) {
            return std::abs(m_formation.center({c, 0}).x - x);
        });
    }
    const Rect alien = m_formation.rect(*m_formation.lowest(column));
    const Vec2 start{alien.center().x, alien.bottom() + m_rules.bombHeight * 0.5f};
    m_bombs.push_back({.type = static_cast<BombType>(type), .position = gamelib::core::Interpolated<Vec2>{start}});
    m_nextBomb = (type + 1) % kBombTypes;
    m_events.push({.kind = EventKind::BombDropped, .position = start, .bomb = static_cast<BombType>(type)});
}

void Session::killCannon()
{
    m_events.push({.kind = EventKind::CannonDestroyed, .position = {m_cannonX.value(), m_rules.cannonY}});
    --m_lives;
    m_bombs.clear();
    m_shot.reset();
    m_phase = Phase::Dying;
    m_phaseTimer.start(ticks(m_rules.deathSeconds));
}

void Session::finishPhase()
{
    if (m_lives <= 0) {
        m_phase = Phase::GameOver;
        if (m_saucer) {
            m_saucer.reset();
            m_events.push({.kind = EventKind::SaucerLeft});
        }
        m_events.push({.kind = EventKind::GameOver});
        return;
    }
    m_cannonX.snap((m_rules.left + m_rules.right) * 0.5f);
    m_phase = Phase::Playing;
    m_events.push({.kind = EventKind::Respawned, .position = {m_cannonX.value(), m_rules.cannonY}});
}

void Session::addScore(const int points)
{
    m_score += points;
    if (!m_extraLifeGiven && m_score >= m_rules.extraLifeAt) {
        m_extraLifeGiven = true;
        ++m_lives;
        m_events.push({.kind = EventKind::ExtraLife});
    }
}

Shield* Session::shieldAt(const Vec2 point)
{
    for (Shield& shield : m_shields) {
        if (shield.bounds().contains(point)) {
            return &shield;
        }
    }
    return nullptr;
}

Rect Session::cannonRect() const
{
    return Rect::fromCenter({m_cannonX.value(), m_rules.cannonY}, {m_rules.cannonWidth, m_rules.cannonHeight});
}

} // namespace invaders::game
