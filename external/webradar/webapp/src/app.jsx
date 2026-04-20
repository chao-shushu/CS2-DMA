import { useEffect, useState, useRef, useMemo } from "react";
import "./App.css";
import PlayerCard from "./components/PlayerCard";
import Radar from "./components/Radar";
import { getLatency, Latency } from "./components/latency";
import MaskedIcon from "./components/maskedicon";
import { getT } from "./utilities/i18n";

const CONNECTION_TIMEOUT = 5000;

/* change this to '1' if you want to use offline (your own pc only) */
const USE_LOCALHOST = 0;

/* you can get your public ip from https://ipinfo.io/ip */
const PUBLIC_IP = "your ip goes here".trim();
const PORT = new URLSearchParams(window.location.search).get("port") || 22006;

const EFFECTIVE_IP = USE_LOCALHOST ? "localhost" : PUBLIC_IP.match(/[a-zA-Z]/) ? window.location.hostname : PUBLIC_IP;

const DEFAULT_SETTINGS = {
  dotSize: 1,
  bombSize: 0.5,
  showAllNames: false,
  showEnemyNames: true,
  showViewCones: false,
  viewConeSize: 1,
  radarZoom: 1,
  showDeadPlayers: true,
  deadPlayerOpacity: 0.4,
  showHealth: false,
  showWeapon: true,
  showPlayerCards: true,
  infoTextSize: 1,
  showBombTimer: true,
  showLatency: true,
  bgOpacity: 0.95,
  smoothTransition: true,
  language: "cn",
};

const loadSettings = () => {
  const savedSettings = localStorage.getItem("radarSettings");
  return savedSettings ? { ...DEFAULT_SETTINGS, ...JSON.parse(savedSettings) } : DEFAULT_SETTINGS;
};

const App = () => {
  const [averageLatency, setAverageLatency] = useState(0);
  const [playerArray, setPlayerArray] = useState([]);
  const [mapData, setMapData] = useState();
  const [localTeam, setLocalTeam] = useState();
  const [bombData, setBombData] = useState();
  const [settings, setSettings] = useState(loadSettings());
  const currentMapRef = useRef(null);
  const t = useMemo(() => getT(settings.language || "cn"), [settings.language]);

  // Save settings to local storage whenever they change
  useEffect(() => {
    localStorage.setItem("radarSettings", JSON.stringify(settings));
  }, [settings]);

  useEffect(() => {
    const webSocketURL = USE_LOCALHOST
      ? `ws://localhost:${PORT}/cs2_webradar`
      : `ws://${EFFECTIVE_IP}:${PORT}/cs2_webradar`;

    let ws = null;
    let reconnectTimer = null;
    let disposed = false;

    const connect = () => {
      if (disposed) return;
      console.info(`[WebRadar] connecting to ${webSocketURL} ...`);

      try { ws = new WebSocket(webSocketURL); } catch (e) {
        console.error("[WebRadar] WebSocket constructor error:", e);
        scheduleReconnect();
        return;
      }

      const connectionTimeout = setTimeout(() => {
        if (ws && ws.readyState === WebSocket.CONNECTING) ws.close();
      }, CONNECTION_TIMEOUT);

      ws.onopen = () => {
        clearTimeout(connectionTimeout);
        console.info("[WebRadar] connected");
        const el = document.getElementsByClassName("radar_message")[0];
        if (el) el.textContent = t("connected");
      };

      ws.onclose = () => {
        clearTimeout(connectionTimeout);
        console.warn("[WebRadar] disconnected, reconnecting in 3s...");
        scheduleReconnect();
      };

      ws.onerror = (error) => {
        clearTimeout(connectionTimeout);
        console.error("[WebRadar] error:", error);
      };

      ws.onmessage = async (event) => {
        try {
          setAverageLatency(getLatency());
          const raw = typeof event.data === 'string' ? event.data : await event.data.text();
          const parsedData = JSON.parse(raw);
          setPlayerArray(parsedData.m_players);
          setLocalTeam(parsedData.m_local_team);
          setBombData(parsedData.m_bomb);

          const map = parsedData.m_map;
          if (map !== "invalid" && map !== currentMapRef.current) {
            currentMapRef.current = map;
            try {
              const mapJson = await (await fetch(`data/${map}/data.json`)).json();
              setMapData({ ...mapJson, name: map });
              document.body.style.backgroundImage = `url(./data/${map}/background.png)`;
            } catch (fetchErr) {
              console.error(`[WebRadar] failed to load map data for ${map}:`, fetchErr);
            }
          }
        } catch (e) {
          console.error("[WebRadar] message parse error:", e);
        }
      };
    };

    const scheduleReconnect = () => {
      if (disposed) return;
      reconnectTimer = setTimeout(connect, 3000);
    };

    connect();

    return () => {
      disposed = true;
      clearTimeout(reconnectTimer);
      if (ws) ws.close();
    };
  }, []);

  return (
    <div className="w-screen h-screen flex flex-col"
      style={{
        background: `radial-gradient(50% 50% at 50% 50%, rgba(20, 40, 55, ${settings.bgOpacity ?? 0.95}) 0%, rgba(7, 20, 30, ${settings.bgOpacity ?? 0.95}) 100%)`,
        backdropFilter: `blur(7.5px)`,
      }}
    >
      <div className={`w-full h-full flex flex-col justify-center overflow-auto relative`}>
        {settings.showBombTimer !== false && bombData && bombData.m_blow_time > 0 && !bombData.m_is_defused && (
          <div className={`absolute left-1/2 -translate-x-1/2 top-2 flex-col items-center gap-1 z-50`}>
            <div className={`flex justify-center items-center gap-1`}>
              <MaskedIcon
                path={`./assets/icons/c4_sml.png`}
                height={32}
                color={
                  (bombData.m_is_defusing &&
                    bombData.m_blow_time - bombData.m_defuse_time > 0 &&
                    `bg-radar-green`) ||
                  (bombData.m_blow_time - bombData.m_defuse_time < 0 &&
                    `bg-radar-red`) ||
                  `bg-radar-secondary`
                }
              />
              <span>{`${bombData.m_blow_time.toFixed(1)}s ${(bombData.m_is_defusing &&
                `(${bombData.m_defuse_time.toFixed(1)}s)`) ||
                ""
                }`}</span>
            </div>
          </div>
        )}

        <div className={`flex items-center justify-evenly`}>
          <Latency
            value={averageLatency}
            settings={settings}
            setSettings={setSettings}
            t={t}
            showLatency={settings.showLatency}
          />

          {settings.showPlayerCards && (
            <ul id="terrorist" className="lg:flex hidden flex-col gap-7 m-0 p-0">
              {playerArray
                .filter((player) => player.m_team == 2)
                .sort((a, b) => a.m_idx - b.m_idx)
                .map((player) => (
                  <PlayerCard
                    isOnRightSide={false}
                    key={player.m_idx}
                    playerData={player}
                    settings={settings}
                    t={t}
                  />
                ))}
            </ul>
          )}

          {(playerArray.length > 0 && mapData && (
            <Radar
              playerArray={playerArray}
              radarImage={`./data/${mapData.name}/radar.png`}
              mapData={mapData}
              localTeam={localTeam}
              averageLatency={averageLatency}
              bombData={bombData}
              settings={settings}
              t={t}
            />
          )) || (
              <div id="radar" className={`relative overflow-hidden origin-center`}>
                <h1 className="radar_message">
                  {t("connecting")}
                </h1>
              </div>
            )}

          {settings.showPlayerCards && (
            <ul
              id="counterTerrorist"
              className="lg:flex hidden flex-col gap-7 m-0 p-0"
            >
              {playerArray
                .filter((player) => player.m_team == 3)
                .sort((a, b) => a.m_idx - b.m_idx)
                .map((player) => (
                  <PlayerCard
                    isOnRightSide={true}
                    key={player.m_idx}
                    playerData={player}
                    settings={settings}
                  />
                ))}
            </ul>
          )}
        </div>
      </div>
    </div>
  );
};

export default App;