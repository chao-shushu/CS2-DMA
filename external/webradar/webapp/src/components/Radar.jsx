import { useRef } from "react";
import Player from "./player";
import Bomb from "./bomb";

const Radar = ({
  playerArray,
  radarImage,
  mapData,
  localTeam,
  averageLatency,
  bombData,
  settings
}) => {
  const radarImageRef = useRef();
  const zoom = settings.radarZoom ?? 1;

  return (
    <div id="radar" className={`relative origin-center flex items-center justify-center`}>
      <div className="relative" style={{
        zoom: zoom,
      }}>
        <img ref={radarImageRef} className={`max-h-[90vh] w-auto h-auto`} src={radarImage} />

        {playerArray.map((player) => (
          <Player
            key={player.m_idx}
            playerData={player}
            mapData={mapData}
            radarImage={radarImageRef.current}
            localTeam={localTeam}
            averageLatency={averageLatency}
            settings={settings}
          />
        ))}

        {bombData && (
          <Bomb
            bombData={bombData}
            mapData={mapData}
            radarImage={radarImageRef.current}
            localTeam={localTeam}
            averageLatency={averageLatency}
            settings={settings}
          />
        )}
      </div>
    </div>
  );
};

export default Radar;