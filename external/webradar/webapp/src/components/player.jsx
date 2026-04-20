import { useRef, useState, useEffect } from "react";
import { getRadarPosition, playerColors } from "../utilities/utilities";


const playerRotations = new Map();
const calculatePlayerRotation = (playerData) => {
  const playerViewAngle = 270 - playerData.m_eye_angle;
  const idx = playerData.m_idx;

  const prev = (playerRotations.get(idx) || 0) % 360;
  const next = prev + (((playerViewAngle - prev + 540) % 360) - 180);
  playerRotations.set(idx, next);

  return next;
};

const Player = ({ playerData, mapData, radarImage, localTeam, averageLatency, settings }) => {
  const [lastKnownPosition, setLastKnownPosition] = useState(null);
  const radarPosition = getRadarPosition(mapData, playerData.m_position) || { x: 0, y: 0 };
  const invalidPosition = radarPosition.x <= 0 && radarPosition.y <= 0;

  const playerRef = useRef();
  const playerBounding = playerRef.current
    ? { width: playerRef.current.offsetWidth, height: playerRef.current.offsetHeight }
    : { width: 0, height: 0 };
  const playerRotation = calculatePlayerRotation(playerData);

  const radarImageBounding = radarImage
    ? { width: radarImage.offsetWidth, height: radarImage.offsetHeight }
    : { width: 0, height: 0 };

  const scaledSize = 0.7 * settings.dotSize;
  const deadOpacity = settings.deadPlayerOpacity ?? 0.4;
  const coneScale = settings.viewConeSize ?? 1;
  const infoScale = settings.infoTextSize ?? 1;

  // Store the last known position when the player dies
  const lastPositionRef = useRef(radarPosition);
  if (!playerData.m_is_dead) {
    lastPositionRef.current = radarPosition;
  }
  useEffect(() => {
    if (playerData.m_is_dead) {
      if (!lastKnownPosition) {
        setLastKnownPosition(lastPositionRef.current);
      }
    } else {
      if (lastKnownPosition) setLastKnownPosition(null);
    }
  }, [playerData.m_is_dead]);

  // Hide dead players if setting is off
  if (playerData.m_is_dead && !settings.showDeadPlayers) return null;

  const effectivePosition = playerData.m_is_dead ? lastKnownPosition || { x: 0, y: 0 } : radarPosition;

  const radarImageTranslation = {
    x: radarImageBounding.width * effectivePosition.x - playerBounding.width * 0.5,
    y: radarImageBounding.height * effectivePosition.y - playerBounding.height * 0.5,
  };

  const showName = (settings.showAllNames && playerData.m_team === localTeam) ||
    (settings.showEnemyNames && playerData.m_team !== localTeam);
  const showWeapon = settings.showWeapon ?? true;
  const activeWeapon = showWeapon && playerData.m_weapons && playerData.m_weapons.m_active;
  const transitionMs = (settings.smoothTransition ?? true) ? averageLatency : 0;

  return (
    <div
      className={`absolute origin-center rounded-[100%] left-0 top-0`}
      ref={playerRef}
      style={{
        width: `${scaledSize}vw`,
        height: `${scaledSize}vw`,
        transform: `translate(${radarImageTranslation.x}px, ${radarImageTranslation.y}px)`,
        transition: transitionMs > 0 ? `transform ${transitionMs}ms linear` : 'none',
        zIndex: `${(playerData.m_is_dead && `0`) || `1`}`,
        WebkitMask: `${(playerData.m_is_dead && `url('./assets/icons/icon-enemy-death_png.png') no-repeat center / contain`) || `none`}`,
      }}
    >
      {/* Name + health + weapon above the dot */}
      {showName ? (
        <div className="absolute bottom-full left-1/2 text-center flex flex-col items-center pointer-events-none"
          style={{ transform: `translateX(-50%) translateY(-0.25rem) scale(${infoScale})`, transformOrigin: "bottom center" }}>
          <span className="text-xs text-white whitespace-nowrap">
            {playerData.m_name}
          </span>
          {settings.showHealth && !playerData.m_is_dead && (
            <span className="text-[10px] text-green-400 leading-none">{playerData.m_health}hp</span>
          )}
          {showWeapon && !playerData.m_is_dead && activeWeapon && (
            <span className="text-[10px] text-yellow-300/80 leading-none">{activeWeapon}</span>
          )}
        </div>
      ) : (
        (!playerData.m_is_dead && (settings.showHealth || (showWeapon && activeWeapon))) ? (
          <div className="absolute bottom-full left-1/2 text-center flex flex-col items-center pointer-events-none"
            style={{ transform: `translateX(-50%) translateY(-0.125rem) scale(${infoScale})`, transformOrigin: "bottom center" }}>
            {settings.showHealth && (
              <span className="text-[10px] text-green-400 leading-none">{playerData.m_health}hp</span>
            )}
            {showWeapon && activeWeapon && (
              <span className="text-[10px] text-yellow-300/80 leading-none">{activeWeapon}</span>
            )}
          </div>
        ) : null
      )}

      {/* Rotating container for player elements */}
      <div
        style={{
          transform: `rotate(${(playerData.m_is_dead && `0`) || playerRotation}deg)`,
          width: `${scaledSize}vw`,
          height: `${scaledSize}vw`,
          transition: transitionMs > 0 ? `transform ${transitionMs}ms linear` : 'none',
          opacity: `${(playerData.m_is_dead && deadOpacity) || (invalidPosition && `0`) || `1`}`,
        }}
      >
        {/* Player dot */}
        <div
          className={`w-full h-full rounded-[50%_50%_50%_0%] rotate-[315deg]`}
          style={{
            backgroundColor: `${(playerData.m_team == localTeam && playerColors[playerData.m_color]) || `red`}`,
            opacity: `${(playerData.m_is_dead && deadOpacity) || (invalidPosition && `0`) || `1`}`,
          }}
        />

        {/* View cone */}
        {settings.showViewCones && !playerData.m_is_dead && (
          <div
            className="absolute left-1/2 top-1/2 bg-white opacity-30"
            style={{
              width: `${1.5 * coneScale}vw`,
              height: `${3 * coneScale}vw`,
              transform: `translate(-50%, 5%) rotate(0deg)`,
              clipPath: "polygon(50% 0%, 0% 100%, 100% 100%)",
            }}
          />
        )}
      </div>
    </div>
  );
};

export default Player;