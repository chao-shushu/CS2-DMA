import { useState } from "react";

const sliderStyle = (value, min, max) => ({
  background: `linear-gradient(to right, #b1d0e7 ${((value - min) / (max - min)) * 100}%, rgba(59, 130, 246, 0.2) ${((value - min) / (max - min)) * 100}%)`
});

const Slider = ({ label, value, min, max, step, suffix, onChange }) => (
  <div>
    <div className="flex justify-between items-center mb-1">
      <span className="text-radar-secondary text-sm">{label}</span>
      <span className="text-radar-primary text-sm font-mono">{value}{suffix || ""}</span>
    </div>
    <input
      type="range" min={min} max={max} step={step} value={value}
      onChange={(e) => onChange(parseFloat(e.target.value))}
      className="w-full h-1.5 rounded-lg appearance-none cursor-pointer"
      style={sliderStyle(value, min, max)}
    />
  </div>
);

const Toggle = ({ label, checked, onChange }) => (
  <label className="flex items-center justify-between py-1.5 px-1 rounded-lg hover:bg-radar-secondary/10 transition-colors cursor-pointer">
    <span className="text-radar-secondary text-sm">{label}</span>
    <input
      type="checkbox" checked={checked}
      onChange={(e) => onChange(e.target.checked)}
      className="relative h-5 w-9 rounded-full shadow-sm bg-radar-secondary/30 checked:bg-radar-secondary transition-colors duration-200 appearance-none before:absolute before:h-4 before:w-4 before:top-0.5 before:left-0.5 before:bg-white before:rounded-full before:transition-transform before:duration-200 checked:before:translate-x-4"
    />
  </label>
);

const Section = ({ title, children }) => (
  <div>
    <div className="text-radar-primary text-xs font-semibold uppercase tracking-wider mb-2 mt-1 opacity-60">{title}</div>
    <div className="space-y-2">{children}</div>
  </div>
);

const LangSwitch = ({ value, onChange, t }) => (
  <div className="flex items-center justify-between py-1.5 px-1">
    <span className="text-radar-secondary text-sm">{t("language")}</span>
    <div className="flex gap-1">
      <button
        onClick={() => onChange("cn")}
        className={`px-2.5 py-0.5 rounded text-xs transition-colors ${value === "cn" ? "bg-radar-secondary text-white" : "bg-radar-secondary/20 text-radar-secondary hover:bg-radar-secondary/30"}`}
      >{t("lang_cn")}</button>
      <button
        onClick={() => onChange("en")}
        className={`px-2.5 py-0.5 rounded text-xs transition-colors ${value === "en" ? "bg-radar-secondary text-white" : "bg-radar-secondary/20 text-radar-secondary hover:bg-radar-secondary/30"}`}
      >{t("lang_en")}</button>
    </div>
  </div>
);

const DEFAULT_SETTINGS = {
  dotSize: 1, bombSize: 0.5,
  showAllNames: false, showEnemyNames: true,
  showViewCones: false, viewConeSize: 1,
  radarZoom: 1, showDeadPlayers: true,
  deadPlayerOpacity: 0.4, showHealth: false,
  showWeapon: true, showPlayerCards: true,
  infoTextSize: 1, showBombTimer: true,
  showLatency: true, bgOpacity: 0.95,
  smoothTransition: true, language: "cn",
};

const SettingsButton = ({ settings, onSettingsChange, t }) => {
  const [isOpen, setIsOpen] = useState(false);
  const s = settings;
  const set = (key) => (val) => onSettingsChange({ ...s, [key]: val });

  return (
    <div className="z-50">
      <button
        onClick={() => setIsOpen(!isOpen)}
        className="flex items-center gap-1 transition-all rounded-xl"
      >
        <img className={`w-[1.3rem]`} src={`./assets/icons/cog.svg`} />
        <span className="text-radar-primary">{t("settings")}</span>
      </button>

      {isOpen && (
        <div className="absolute right-0 mt-2 w-72 max-h-[80vh] overflow-y-auto bg-radar-panel/95 backdrop-blur-lg rounded-xl p-4 shadow-xl border border-radar-secondary/20">
          <div className="flex justify-between items-center mb-3">
            <h3 className="text-radar-primary text-base font-semibold">{t("settings")}</h3>
            <button
              onClick={() => onSettingsChange({ ...DEFAULT_SETTINGS, language: s.language })}
              className="text-radar-secondary text-xs hover:text-radar-primary transition-colors"
            >{t("reset")}</button>
          </div>

          <div className="space-y-4">
            <Section title={t("sec_radar")}>
              <Slider label={t("zoom")} value={s.radarZoom} min={0.5} max={3} step={0.1} suffix="x" onChange={set("radarZoom")} />
              <Slider label={t("dotSize")} value={s.dotSize} min={0.3} max={3} step={0.1} suffix="x" onChange={set("dotSize")} />
              <Slider label={t("bombSize")} value={s.bombSize} min={0.1} max={3} step={0.1} suffix="x" onChange={set("bombSize")} />
            </Section>

            <Section title={t("sec_players")}>
              <Toggle label={t("teamNames")} checked={s.showAllNames} onChange={set("showAllNames")} />
              <Toggle label={t("enemyNames")} checked={s.showEnemyNames} onChange={set("showEnemyNames")} />
              <Toggle label={t("showHealth")} checked={s.showHealth} onChange={set("showHealth")} />
              <Toggle label={t("showWeapon")} checked={s.showWeapon ?? true} onChange={set("showWeapon")} />
              <Slider label={t("infoTextSize")} value={s.infoTextSize ?? 1} min={0.5} max={3} step={0.1} suffix="x" onChange={set("infoTextSize")} />
              <Toggle label={t("showDeadPlayers")} checked={s.showDeadPlayers} onChange={set("showDeadPlayers")} />
              {s.showDeadPlayers && (
                <Slider label={t("deadOpacity")} value={s.deadPlayerOpacity} min={0.1} max={1} step={0.05} suffix="" onChange={set("deadPlayerOpacity")} />
              )}
            </Section>

            <Section title={t("sec_display")}>
              <Toggle label={t("viewCones")} checked={s.showViewCones} onChange={set("showViewCones")} />
              {s.showViewCones && (
                <Slider label={t("coneSize")} value={s.viewConeSize} min={0.3} max={3} step={0.1} suffix="x" onChange={set("viewConeSize")} />
              )}
              <Toggle label={t("playerCards")} checked={s.showPlayerCards} onChange={set("showPlayerCards")} />
              <Toggle label={t("showBombTimer")} checked={s.showBombTimer ?? true} onChange={set("showBombTimer")} />
              <Toggle label={t("showLatency")} checked={s.showLatency ?? true} onChange={set("showLatency")} />
              <Toggle label={t("smoothing")} checked={s.smoothTransition ?? true} onChange={set("smoothTransition")} />
              <Slider label={t("bgOpacity")} value={s.bgOpacity ?? 0.95} min={0} max={1} step={0.05} suffix="" onChange={set("bgOpacity")} />
            </Section>

            <Section title={t("sec_system")}>
              <LangSwitch value={s.language || "cn"} onChange={set("language")} t={t} />
            </Section>
          </div>
        </div>
      )}
    </div>
  );
};

export default SettingsButton;