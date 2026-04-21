#pragma once
#include <string>

class Language {
public:
	std::string tab_visuals;
	std::string tab_radar;
	std::string tab_settings;
	std::string tab_config;
	std::string tab_grenade;
	std::string tab_fusion;

	std::string visuals_showbox;
	std::string visuals_boxcolor;
	std::string visuals_boxtype;
	std::string visuals_showbone;
	std::string visuals_bonecolor;
	std::string visuals_showeyeray;
	std::string visuals_eyeraycolor;
	std::string visuals_showbar;
	std::string visuals_barpos;
	std::string visuals_weaponesp;
	std::string visuals_distance;
	std::string visuals_name;
	std::string visuals_line;
	std::string visuals_linecolor;

	std::string utilities_teamcheck;
	std::string utilities_closehack;
	std::string utilities_reloadhack;
	std::string utilities_language;
	std::string utilities_help;

	std::string config_newconfig;
	std::string config_create;
	std::string config_load;
	std::string config_save;
	std::string config_delete;

	const char* visuals_boxtypeselect[3];
	const char* visuals_heathbarselect[3];

	const char* utilities_langselect[2] = { "English", u8"\u4e2d\u6587" };

	std::string days;

	std::string frames;

	// Grenade Helper
	std::string grenade_enable;
	std::string grenade_showname;
	std::string grenade_showbox;
	std::string grenade_showline;
	std::string grenade_autoaim;
	std::string grenade_maxdistance;
	std::string grenade_triggerdistance;
	std::string grenade_boxsize;
	std::string grenade_currentmap;
	std::string grenade_availablethrows;
	std::string grenade_nomapdata;
	std::string grenade_flash;
	std::string grenade_smoke;
	std::string grenade_he;
	std::string grenade_molotov;

	// Grenade Recording
	std::string grenade_record_hotkey;
	std::string grenade_pending_throws;
	std::string grenade_name_throw;
	std::string grenade_throw_name;
	std::string grenade_throw_style;
	std::string grenade_throw_type;
	std::string grenade_save_throws;
	std::string grenade_delete;
	std::string grenade_clear_all;
	std::string grenade_no_pending;
	std::string grenade_position;
	std::string grenade_angle;
	std::string grenade_recorded_at;

	// Section Headers
	std::string header_playerbox;
	std::string header_skeleton;
	std::string header_health;
	std::string header_info;
	std::string header_snapline;
	std::string header_general;
	std::string header_system;
	std::string header_display;
	std::string header_recording;
	std::string header_pending;
	std::string header_savededitor;
	std::string header_spectator;

	// Visuals Extra
	std::string visuals_thickness;
	std::string visuals_rounding;
	std::string visuals_cornersize;
	std::string visuals_filled;
	std::string visuals_fillalpha;
	std::string visuals_headdot;
	std::string visuals_dotsize;
	std::string visuals_length;
	std::string visuals_barwidth;
	std::string visuals_armorbar;
	std::string visuals_size;
	std::string visuals_origin;
	std::string visuals_width;

	const char* armorbar_typeselect[2];
	const char* snapline_originselect[3];

	// Safe Zone
	std::string header_safezone;
	std::string safezone_enable;
	std::string safezone_radius;
	std::string safezone_shape;
	const char* safezone_shapeselect[2];

	// Spectator List & Perf Monitor
	std::string visuals_spectatorlist;
	std::string settings_perfmonitor;

	// Debug
	std::string settings_debuglog;
	std::string settings_debuglog_tip;

	// Settings
	std::string settings_vsync;
	std::string settings_maxfps;
	std::string settings_unlimited;
	std::string settings_unlimitedtip;
	std::string settings_restarttip;

	// Display / Resolution
	std::string settings_resolution;
	std::string settings_renderautotip;
	std::string settings_monitor;
	std::string settings_monitortip;

	// Grenade Extra
	std::string grenade_pressanykey;
	std::string grenade_hotkeytip;
	std::string grenade_autosave;
	std::string grenade_autosavetip;
	std::string grenade_defaulttype;
	std::string grenade_defaultstyle;
	std::string grenade_reloadfiles;
	std::string grenade_reloadtip;
	std::string grenade_selectmap;
	std::string grenade_totalthrows;
	std::string grenade_editthrow;
	std::string grenade_update;
	std::string grenade_nomaps;
	std::string grenade_name_label;

	const char* grenade_typeselect[4];
	const char* grenade_styleselect[5];

	// Grenade Type/Style Display Names
	std::string grenade_typename_flash;
	std::string grenade_typename_smoke;
	std::string grenade_typename_he;
	std::string grenade_typename_fire;
	std::string grenade_typename_unknown;
	std::string grenade_stylename_stand;
	std::string grenade_stylename_run;
	std::string grenade_stylename_jump;
	std::string grenade_stylename_crouch;
	std::string grenade_stylename_runjump;
	std::string grenade_stylename_unknown;

	// Direction Labels
	std::string dir_forward;
	std::string dir_back;
	std::string dir_right;
	std::string dir_left;
	std::string dir_up;
	std::string dir_down;
	std::string dir_f;
	std::string dir_b;
	std::string dir_l;
	std::string dir_r;

	// Status Messages
	std::string status_dma_init;
	std::string status_dma_failed;
	std::string status_searching;
	std::string status_init_game;
	std::string status_unknown;

	// Projectile ESP
	std::string proj_header;
	std::string proj_enable;
	std::string proj_range;
	std::string proj_rangealpha;

	// Crosshair Overlay
	std::string header_crosshair;
	std::string crosshair_enable;
	std::string crosshair_size;
	std::string crosshair_thickness;
	std::string crosshair_gap;
	std::string crosshair_style;
	std::string crosshair_color;
	std::string crosshair_onenemycolor;
	std::string crosshair_enemycolor;
	const char* crosshair_styleselect[4];

	Language() { english(); }

	void english() {
		this->tab_visuals = "Visuals";
		this->tab_radar = "Radar";
		this->tab_settings = "Settings";
		this->tab_config = "Config";
		this->tab_grenade = "Grenade";
		this->tab_fusion = "Fusion";

		this->visuals_showbox = "Show Box";
		this->visuals_boxcolor = "Box Color";
		this->visuals_boxtype = "Box Type";
		this->visuals_showbone = "Bones";
		this->visuals_bonecolor = "Bones Color";
		this->visuals_showeyeray = "Eye Ray";
		this->visuals_eyeraycolor = "Eye Ray Color";
		this->visuals_showbar = "Health Bar";
		this->visuals_weaponesp = "Weapon";
		this->visuals_distance = "Distance";
		this->visuals_name = "Name";
		this->visuals_line = "Lines to Player";
		this->visuals_linecolor = "Lines color";

		this->utilities_teamcheck = "Team Check";
		this->utilities_closehack = "Close Software";
		this->utilities_reloadhack = "Reload Game (Click if not working)";
		this->utilities_language = "Select Language";
		this->utilities_help = "Help";

		this->config_newconfig = "Config Name";
		this->config_create = "Create config";
		this->config_load = "Load";
		this->config_save = "Save";
		this->config_delete = "Delete";

		this->visuals_boxtypeselect[0] = "Normal"; this->visuals_boxtypeselect[1] = "Slim"; this->visuals_boxtypeselect[2] = "Corner";
		this->visuals_heathbarselect[0] = "Left"; this->visuals_heathbarselect[1] = "Up"; this->visuals_heathbarselect[2] = "Number";

		this->days = "days";

		this->frames = "Frames";

		// Grenade Helper
		this->grenade_enable = "Enable Grenade Helper";
		this->grenade_showname = "Show Name";
		this->grenade_showbox = "Show Box";
		this->grenade_showline = "Show Line";
		this->grenade_autoaim = "Auto Aim";
		this->grenade_maxdistance = "Max Distance";
		this->grenade_triggerdistance = "Trigger Distance";
		this->grenade_boxsize = "Box Size";
		this->grenade_currentmap = "Current Map";
		this->grenade_availablethrows = "Available Throws";
		this->grenade_nomapdata = "No data for current map";
		this->grenade_flash = "Flash";
		this->grenade_smoke = "Smoke";
		this->grenade_he = "HE";
		this->grenade_molotov = "Molotov";

		// Grenade Recording
		this->grenade_record_hotkey = "Record Hotkey";
		this->grenade_pending_throws = "Pending Throws";
		this->grenade_name_throw = "Name Throw";
		this->grenade_throw_name = "Throw Name";
		this->grenade_throw_style = "Style";
		this->grenade_throw_type = "Type";
		this->grenade_save_throws = "Save to File";
		this->grenade_delete = "Delete";
		this->grenade_clear_all = "Clear All";
		this->grenade_no_pending = "No pending throws";
		this->grenade_position = "Position";
		this->grenade_angle = "Angle";
		this->grenade_recorded_at = "Recorded at";

		this->header_playerbox = "Player Box / ESP";
		this->header_skeleton = "Skeleton / Eye Ray";
		this->header_health = "Health & Armor";
		this->header_info = "Info & Text";
		this->header_snapline = "Snapline";
		this->header_general = "General";
		this->header_system = "System";
		this->header_recording = "Recording / Hotkey";
		this->header_pending = "Pending Throws";
		this->header_savededitor = "Saved Throws Editor";
		this->header_spectator = "Spectator List";

		this->visuals_thickness = "Thickness";
		this->visuals_rounding = "Rounding";
		this->visuals_cornersize = "Corner Size";
		this->visuals_filled = "Filled";
		this->visuals_fillalpha = "Fill Alpha";
		this->visuals_headdot = "Head Dot";
		this->visuals_dotsize = "Dot Size";
		this->visuals_length = "Length";
		this->visuals_barwidth = "Bar Width";
		this->visuals_armorbar = "Armor Bar";
		this->visuals_size = "Size";
		this->visuals_origin = "Origin";
		this->visuals_width = "Width";

		this->armorbar_typeselect[0] = "Right"; this->armorbar_typeselect[1] = "Top";
		this->snapline_originselect[0] = "Top"; this->snapline_originselect[1] = "Center"; this->snapline_originselect[2] = "Bottom";

		this->header_safezone = "Crosshair Safe Zone";
		this->safezone_enable = "Enable Safe Zone";
		this->safezone_radius = "Radius";
		this->safezone_shape = "Shape";
		this->safezone_shapeselect[0] = "Circle"; this->safezone_shapeselect[1] = "Square";

		this->visuals_spectatorlist = "Show Spectator List";
		this->settings_perfmonitor = "Performance Monitor";

		this->settings_debuglog = "Debug Log";
		this->settings_debuglog_tip = "Enable verbose TRACE/DEBUG logging for troubleshooting (impacts performance)";

		this->settings_vsync = "VSync";
		this->settings_maxfps = "Max FPS";
		this->settings_unlimited = "Unlimited";
		this->settings_unlimitedtip = "0 = Unlimited";
		this->settings_restarttip = "restart to apply";

		this->settings_resolution = "Resolution";
		this->settings_renderautotip = "Auto = use monitor native resolution, restart to apply";
		this->settings_monitor = "Monitor";
		this->settings_monitortip = "Select which monitor to render on, restart to apply";

		this->grenade_pressanykey = "Press any key...";
		this->grenade_hotkeytip = "Click button then press key | Supports mouse side buttons | ESC cancel";
		this->grenade_autosave = "Auto Save";
		this->grenade_autosavetip = "Automatically save throws after recording";
		this->grenade_defaulttype = "Default Type";
		this->grenade_defaultstyle = "Default Style";
		this->grenade_reloadfiles = "Reload Files";
		this->grenade_reloadtip = "Reload all grenade helper data from JSON files";
		this->grenade_selectmap = "Select Map";
		this->grenade_totalthrows = "Total throws";
		this->grenade_editthrow = "Edit Throw";
		this->grenade_update = "Update";
		this->grenade_nomaps = "No maps loaded";
		this->grenade_name_label = "Name";

		this->grenade_typeselect[0] = "Flash"; this->grenade_typeselect[1] = "Smoke"; this->grenade_typeselect[2] = "HE"; this->grenade_typeselect[3] = "Molotov";
		this->grenade_styleselect[0] = "Stand"; this->grenade_styleselect[1] = "Run"; this->grenade_styleselect[2] = "Jump"; this->grenade_styleselect[3] = "Crouch"; this->grenade_styleselect[4] = "Run+Jump";

		this->grenade_typename_flash = "flash";
		this->grenade_typename_smoke = "smoke";
		this->grenade_typename_he = "HE";
		this->grenade_typename_fire = "fire";
		this->grenade_typename_unknown = "unknown";
		this->grenade_stylename_stand = "stand";
		this->grenade_stylename_run = "run";
		this->grenade_stylename_jump = "jump";
		this->grenade_stylename_crouch = "crouch";
		this->grenade_stylename_runjump = "run+jump";
		this->grenade_stylename_unknown = "unknown";

		this->dir_forward = "Forward"; this->dir_back = "Back";
		this->dir_right = "Right"; this->dir_left = "Left";
		this->dir_up = "Up"; this->dir_down = "Down";
		this->dir_f = "F"; this->dir_b = "B"; this->dir_l = "L"; this->dir_r = "R";

		this->proj_header = "Grenade Projectile ESP";
		this->proj_enable = "Show Projectile ESP";
		this->proj_range = "Show Effect Range";
		this->proj_rangealpha = "Range Alpha";

		this->header_crosshair = "Crosshair Overlay";
		this->crosshair_enable = "Enable Crosshair";
		this->crosshair_size = "Arm Length";
		this->crosshair_thickness = "Thickness";
		this->crosshair_gap = "Gap";
		this->crosshair_style = "Style";
		this->crosshair_color = "Color";
		this->crosshair_onenemycolor = "Change Color on Enemy";
		this->crosshair_enemycolor = "Enemy Color";
		this->crosshair_styleselect[0] = "Cross"; this->crosshair_styleselect[1] = "Dot"; this->crosshair_styleselect[2] = "Circle"; this->crosshair_styleselect[3] = "Cross+Dot";

		this->status_dma_init = "Initializing DMA...";
		this->status_dma_failed = "DMA Connection Failed!";
		this->status_searching = "Searching for cs2.exe...";
		this->status_init_game = "Initializing game data...";
		this->status_unknown = "Unknown state";
	}

	void chineese() {
		this->tab_visuals = u8"\u89c6\u89c9";
		this->tab_radar = u8"\u96f7\u8fbe";
		this->tab_settings = u8"\u8bbe\u7f6e";
		this->tab_config = u8"\u914d\u7f6e";
		this->tab_grenade = u8"\u6295\u63b7\u7269";
		this->tab_fusion = u8"\u878d\u5408\u5668\u4f18\u5316";

		this->visuals_showbox = u8"\u663e\u793a\u900f\u89c6\u6846";
		this->visuals_boxcolor = u8"\u6846\u989c\u8272";
		this->visuals_boxtype = u8"\u6846\u7c7b\u578b";
		this->visuals_showbone = u8"\u663e\u793a\u9aa8\u9abc";
		this->visuals_bonecolor = u8"\u9aa8\u9abc\u989c\u8272";
		this->visuals_showeyeray = u8"\u663e\u793a\u89c6\u7ebf";
		this->visuals_eyeraycolor = u8"\u89c6\u7ebf\u989c\u8272";
		this->visuals_showbar = u8"\u663e\u793a\u751f\u547d\u6761";
		this->visuals_weaponesp = u8"\u663e\u793a\u6b66\u5668";
		this->visuals_distance = u8"\u663e\u793a\u8ddd\u79bb";
		this->visuals_name = u8"\u663e\u793a\u540d\u79f0";
		this->visuals_line = u8"\u663e\u793a\u6307\u5411\u73a9\u5bb6\u7684\u7ebf";
		this->visuals_linecolor = u8"\u7ebf\u989c\u8272";

		this->utilities_teamcheck = u8"\u961f\u4f0d\u68c0\u67e5";
		this->utilities_closehack = u8"\u5173\u95ed\u8f6f\u4ef6";
		this->utilities_language = u8"\u9009\u62e9\u8bed\u8a00";
		this->utilities_reloadhack = u8"\u91cd\u65b0\u8bfb\u53d6\u6e38\u620f(\u65e0\u6548\u65f6\u70b9\u6211)";
		this->utilities_help = u8"\u5e2e\u52a9";

		this->config_newconfig = u8"\u914d\u7f6e\u540d\u79f0";
		this->config_create = u8"\u521b\u5efa\u914d\u7f6e";
		this->config_load = u8"\u52a0\u8f7d";
		this->config_save = u8"\u4fdd\u5b58";
		this->config_delete = u8"\u5220\u9664";

		this->visuals_boxtypeselect[0] = u8"\u666e\u901a"; this->visuals_boxtypeselect[1] = u8"\u7a84"; this->visuals_boxtypeselect[2] = u8"\u62d0\u89d2";
		this->visuals_heathbarselect[0] = u8"\u5de6\u4fa7"; this->visuals_heathbarselect[1] = u8"\u4e0a\u4fa7"; this->visuals_heathbarselect[2] = u8"\u6570\u5b57";

		this->days = u8"\u5269\u4f59\u5929\u6570";

		this->frames = u8"\u6846";

		// Grenade Helper
		this->grenade_enable = u8"\u542f\u7528\u6295\u63b7\u7269\u8f85\u52a9";
		this->grenade_showname = u8"\u663e\u793a\u540d\u79f0";
		this->grenade_showbox = u8"\u663e\u793a\u65b9\u6846";
		this->grenade_showline = u8"\u663e\u793a\u8fde\u7ebf";
		this->grenade_autoaim = u8"\u81ea\u52a8\u7784\u51c6";
		this->grenade_maxdistance = u8"\u6700\u5927\u8ddd\u79bb";
		this->grenade_triggerdistance = u8"\u89e6\u53d1\u8ddd\u79bb";
		this->grenade_boxsize = u8"\u65b9\u6846\u5927\u5c0f";
		this->grenade_currentmap = u8"\u5f53\u524d\u5730\u56fe";
		this->grenade_availablethrows = u8"\u53ef\u7528\u6295\u63b7\u70b9";
		this->grenade_nomapdata = u8"\u5f53\u524d\u5730\u56fe\u65e0\u6570\u636e";
		this->grenade_flash = u8"\u95ea\u5149";
		this->grenade_smoke = u8"\u70df\u96fe";
		this->grenade_he = u8"\u624b\u96f7";
		this->grenade_molotov = u8"\u71c3\u70e7";

		// Grenade Recording
		this->grenade_record_hotkey = u8"\u8bb0\u5f55\u5feb\u6301\u952e";
		this->grenade_pending_throws = u8"\u5f85\u547d\u540d\u70b9\u4f4d";
		this->grenade_name_throw = u8"\u547d\u540d\u70b9\u4f4d";
		this->grenade_throw_name = u8"\u70b9\u4f4d\u540d\u79f0";
		this->grenade_throw_style = u8"\u6295\u63b7\u65b9\u5f0f";
		this->grenade_throw_type = u8"\u6295\u63b7\u7c7b\u578b";
		this->grenade_save_throws = u8"\u4fdd\u5b58\u5230\u6587\u4ef6";
		this->grenade_delete = u8"\u5220\u9664";
		this->grenade_clear_all = u8"\u6e05\u7a81\u6240\u6709";
		this->grenade_no_pending = u8"\u6ca1\u6709\u5f85\u547d\u540d\u70b9\u4f4d";
		this->grenade_position = u8"\u4f4d\u7f6e";
		this->grenade_angle = u8"\u89d2\u5ea6";
		this->grenade_recorded_at = u8"\u8bb0\u5f55\u65f6\u95f4";

		this->header_playerbox = u8"\u65b9\u6846\u900f\u89c6";
		this->header_skeleton = u8"\u9aa8\u9abc / \u89c6\u7ebf";
		this->header_health = u8"\u751f\u547d\u503c & \u62a4\u7532";
		this->header_info = u8"\u4fe1\u606f & \u6587\u5b57";
		this->header_snapline = u8"\u5c04\u7ebf";
		this->header_general = u8"\u5e38\u89c4";
		this->header_system = u8"\u7cfb\u7edf";
		this->header_recording = u8"\u5f55\u5236 / \u5feb\u6377\u952e";
		this->header_pending = u8"\u5f85\u547d\u540d\u6295\u63b7\u70b9";
		this->header_savededitor = u8"\u5df2\u4fdd\u5b58\u6295\u63b7\u70b9\u7f16\u8f91";
		this->header_spectator = u8"\u89c2\u4f17\u5217\u8868";

		this->visuals_thickness = u8"\u7c97\u7ec6";
		this->visuals_rounding = u8"\u5706\u89d2";
		this->visuals_cornersize = u8"\u62d0\u89d2\u5927\u5c0f";
		this->visuals_filled = u8"\u586b\u5145";
		this->visuals_fillalpha = u8"\u586b\u5145\u900f\u660e\u5ea6";
		this->visuals_headdot = u8"\u5934\u90e8\u5706\u70b9";
		this->visuals_dotsize = u8"\u5706\u70b9\u5927\u5c0f";
		this->visuals_length = u8"\u957f\u5ea6";
		this->visuals_barwidth = u8"\u8840\u6761\u5bbd\u5ea6";
		this->visuals_armorbar = u8"\u62a4\u7532\u6761";
		this->visuals_size = u8"\u5927\u5c0f";
		this->visuals_origin = u8"\u8d77\u70b9";
		this->visuals_width = u8"\u5bbd\u5ea6";

		this->armorbar_typeselect[0] = u8"\u53f3\u4fa7"; this->armorbar_typeselect[1] = u8"\u4e0a\u65b9";
		this->snapline_originselect[0] = u8"\u4e0a\u65b9"; this->snapline_originselect[1] = u8"\u4e2d\u95f4"; this->snapline_originselect[2] = u8"\u4e0b\u65b9";

		this->header_safezone = u8"\u51c6\u661f\u5b89\u5168\u533a";
		this->safezone_enable = u8"\u542f\u7528\u5b89\u5168\u533a";
		this->safezone_radius = u8"\u534a\u5f84";
		this->safezone_shape = u8"\u5f62\u72b6";
		this->safezone_shapeselect[0] = u8"\u5706\u5f62"; this->safezone_shapeselect[1] = u8"\u65b9\u5f62";

		this->visuals_spectatorlist = u8"\u663e\u793a\u89c2\u4f17\u5217\u8868";
		this->settings_perfmonitor = u8"\u6027\u80fd\u76d1\u63a7";

		this->settings_debuglog = u8"\u8c03\u8bd5\u65e5\u5fd7";
		this->settings_debuglog_tip = u8"\u542f\u7528\u8be6\u7ec6\u7684 TRACE/DEBUG \u65e5\u5fd7\u8f93\u51fa\u7528\u4e8e\u95ee\u9898\u5b9a\u4f4d\uff08\u5f71\u54cd\u6027\u80fd\uff09";

		this->settings_vsync = u8"\u5782\u76f4\u540c\u6b65";
		this->settings_maxfps = u8"\u6700\u5927\u5e27\u7387";
		this->settings_unlimited = u8"\u65e0\u9650\u5236";
		this->settings_unlimitedtip = u8"0 = \u65e0\u9650\u5236";
		this->settings_restarttip = u8"\u91cd\u542f\u540e\u751f\u6548";

		this->settings_resolution = u8"\u5206\u8fa8\u7387";
		this->settings_renderautotip = u8"\u81ea\u52a8 = \u4f7f\u7528\u663e\u793a\u5668\u539f\u751f\u5206\u8fa8\u7387, \u91cd\u542f\u540e\u751f\u6548";
		this->settings_monitor = u8"\u663e\u793a\u5668";
		this->settings_monitortip = u8"\u9009\u62e9\u7ed8\u5236\u7684\u663e\u793a\u5668, \u91cd\u542f\u540e\u751f\u6548";

		this->grenade_pressanykey = u8"\u6309\u4efb\u610f\u952e...";
		this->grenade_hotkeytip = u8"\u70b9\u51fb\u6309\u94ae\u540e\u6309\u952e | \u652f\u6301\u9f20\u6807\u4fa7\u952e | ESC\u53d6\u6d88";
		this->grenade_autosave = u8"\u81ea\u52a8\u4fdd\u5b58";
		this->grenade_autosavetip = u8"\u5f55\u5236\u540e\u81ea\u52a8\u4fdd\u5b58\u6295\u63b7\u70b9";
		this->grenade_defaulttype = u8"\u9ed8\u8ba4\u7c7b\u578b";
		this->grenade_defaultstyle = u8"\u9ed8\u8ba4\u65b9\u5f0f";
		this->grenade_reloadfiles = u8"\u91cd\u65b0\u52a0\u8f7d";
		this->grenade_reloadtip = u8"\u4ece JSON \u6587\u4ef6\u91cd\u65b0\u52a0\u8f7d\u6240\u6709\u6295\u63b7\u7269\u6570\u636e";
		this->grenade_selectmap = u8"\u9009\u62e9\u5730\u56fe";
		this->grenade_totalthrows = u8"\u603b\u6295\u63b7\u70b9";
		this->grenade_editthrow = u8"\u7f16\u8f91\u6295\u63b7\u70b9";
		this->grenade_update = u8"\u66f4\u65b0";
		this->grenade_nomaps = u8"\u65e0\u5730\u56fe\u6570\u636e";
		this->grenade_name_label = u8"\u540d\u79f0";

		this->grenade_typeselect[0] = u8"\u95ea\u5149"; this->grenade_typeselect[1] = u8"\u70df\u96fe"; this->grenade_typeselect[2] = u8"\u624b\u96f7"; this->grenade_typeselect[3] = u8"\u71c3\u70e7";
		this->grenade_styleselect[0] = u8"\u7ad9\u7acb"; this->grenade_styleselect[1] = u8"\u8dd1\u52a8"; this->grenade_styleselect[2] = u8"\u8df3\u8dc3"; this->grenade_styleselect[3] = u8"\u8e72\u4e0b"; this->grenade_styleselect[4] = u8"\u8dd1\u8df3";

		this->grenade_typename_flash = u8"\u95ea\u5149";
		this->grenade_typename_smoke = u8"\u70df\u96fe";
		this->grenade_typename_he = u8"\u624b\u96f7";
		this->grenade_typename_fire = u8"\u71c3\u70e7";
		this->grenade_typename_unknown = u8"\u672a\u77e5";
		this->grenade_stylename_stand = u8"\u7ad9\u7acb";
		this->grenade_stylename_run = u8"\u8dd1\u52a8";
		this->grenade_stylename_jump = u8"\u8df3\u8dc3";
		this->grenade_stylename_crouch = u8"\u8e72\u4e0b";
		this->grenade_stylename_runjump = u8"\u8dd1\u8df3";
		this->grenade_stylename_unknown = u8"\u672a\u77e5";

		this->dir_forward = u8"\u524d\u65b9"; this->dir_back = u8"\u540e\u65b9";
		this->dir_right = u8"\u53f3\u65b9"; this->dir_left = u8"\u5de6\u65b9";
		this->dir_up = u8"\u4e0a\u65b9"; this->dir_down = u8"\u4e0b\u65b9";
		this->dir_f = u8"\u524d"; this->dir_b = u8"\u540e"; this->dir_l = u8"\u5de6"; this->dir_r = u8"\u53f3";

		this->proj_header = u8"\u6295\u63b7\u7269\u5b9e\u65f6ESP";
		this->proj_enable = u8"\u663e\u793a\u6295\u63b7\u7269ESP";
		this->proj_range = u8"\u663e\u793a\u751f\u6548\u8303\u56f4";
		this->proj_rangealpha = u8"\u8303\u56f4\u900f\u660e\u5ea6";

		this->header_crosshair = u8"\u51c6\u661f\u8986\u76d6\u5c42";
		this->crosshair_enable = u8"\u542f\u7528\u51c6\u661f";
		this->crosshair_size = u8"\u81c2\u957f";
		this->crosshair_thickness = u8"\u7c97\u7ec6";
		this->crosshair_gap = u8"\u95f4\u8ddd";
		this->crosshair_style = u8"\u6837\u5f0f";
		this->crosshair_color = u8"\u989c\u8272";
		this->crosshair_onenemycolor = u8"\u7784\u51c6\u654c\u4eba\u53d8\u8272";
		this->crosshair_enemycolor = u8"\u654c\u4eba\u989c\u8272";
		this->crosshair_styleselect[0] = u8"\u5341\u5b57"; this->crosshair_styleselect[1] = u8"\u5706\u70b9"; this->crosshair_styleselect[2] = u8"\u5706\u5708"; this->crosshair_styleselect[3] = u8"\u5341\u5b57+\u5706\u70b9";

		this->status_dma_init = u8"\u521d\u59cb\u5316DMA...";
		this->status_dma_failed = u8"DMA\u8fde\u63a5\u5931\u8d25!";
		this->status_searching = u8"\u641c\u7d22cs2.exe\u4e2d...";
		this->status_init_game = u8"\u521d\u59cb\u5316\u6e38\u620f\u6570\u636e\u4e2d...";
		this->status_unknown = u8"\u672a\u77e5\u72b6\u6001";
	}
};

inline Language lang;