# SAR: Cvars (Pre-release only)

|Name|Default|Description|
|---|---|---|
|cl_crosshair_t|0|Removes the top line from the crosshair :0 : normal crosshair,1 : crosshair without top.<br>|
|cl_crosshairalpha|255|Change the amount of transparency.<br>|
|cl_crosshaircolor_b|0|Changes the color of the crosshair.<br>|
|cl_crosshaircolor_g|255|Changes the color of the crosshair.<br>|
|cl_crosshaircolor_r|0|Changes the color of the crosshair.<br>|
|cl_crosshairdot|1|Decides if there is a dot in the middle of the crosshair<br>|
|cl_crosshairgap|5|Changes the distance of the crosshair lines from the center of screen.<br>|
|cl_crosshairsize|5|Changes the size of the crosshair.<br>|
|cl_crosshairthickness|0|Changes the thinkness of the crosshair lines.<br>|
|cl_quickhud_alpha|255|Change the amount of transparency.<br>|
|cl_quickhudleftcolor_b|86|Changes the color of the left quickhud.<br>|
|cl_quickhudleftcolor_g|184|Changes the color of the left quickhud.<br>|
|cl_quickhudleftcolor_r|255|Changes the color of the left quickhud.<br>|
|cl_quickhudrightcolor_b|255|Changes the color of the right quickhud.<br>|
|cl_quickhudrightcolor_g|184|Changes the color of the right quickhud.<br>|
|cl_quickhudrightcolor_r|111|Changes the color of the right quickhud.<br>|
|ghost_connect|cmd|Connect to server.<br>|
|ghost_delete_all|cmd|Delete all ghosts.<br>|
|ghost_delete_by_ID|cmd|ghost_delete_by_ID \<ID>. Delete the ghost selected.<br>|
|ghost_disconnect|cmd|Disconnect.<br>|
|ghost_height|16|Height of the ghosts.<br>|
|ghost_message|cmd|Send message|
|ghost_name|cmd|Change your name.<br>|
|ghost_offset|cmd|ghost_offset \<offset> \<ID>. Delay the ghost start by \<offset> frames.<br>|
|ghost_ping|cmd|Pong !|
|ghost_prop_model|cmd|Set the prop model. Example : models/props/metal_box.mdl<br>|
|ghost_recap|cmd|Recap all ghosts setup.<br>|
|ghost_set_demo|cmd|ghost_set_demo \<demo> [ID]. Ghost will use this demo. If ID is specified, will create or modify the ID-�me ghost.<br>|
|ghost_set_demos|cmd|ghost_set_demos \<first_demo> [ID]. Ghost will setup a speedrun with first_demo, first_demo_2, etc.<br>If ID is specified, will create or modify the ID-th ghost.<br>|
|ghost_show_advancement|1|Show the advancement of the ghosts.<br>|
|ghost_show_difference|0|Display time difference between players after they load a map.<br>|
|ghost_start|cmd|Start ghosts|
|ghost_stop|cmd|Reset ghosts.<br>|
|ghost_sync|0|When loading a new level, pauses the game until other players load it.<br>|
|ghost_TCP_only|0|Lathil's special command :).<br>|
|ghost_text_offset|20|Offset of the name over the ghosts.<br>|
|ghost_transparency|255|Transparency of the ghosts.<br>|
|ghost_type|cmd|ghost_type \<0/1>:<br>0: Ghost not recorded in demos.<br>1: Ghost using props model but recorded in demos (NOT RECOMMANDED !).<br>|
|ghost_update_rate|50|Adjust the update rate. For people with lathil's internet.<br>|
|sar_crosshair_mode|0|Set the crosshair mode :<br>0 : Default crosshair<br>1 : Customizable crosshair<br>2 : Crosshair from .png<br>|
|sar_crosshair_set_texture|cmd|sar_crosshair_set_texture \<filepath><br>|
|sar_export_stats|cmd|sar_export_stats [filePath]. Export the stats to the specifed path in a .csv file.<br>|
|sar_hud_ghost_show_name|1|Display the name of the ghost over it.<br>|
|sar_hud_strafesync_color|0 150 250 255|RGBA font color of strafesync HUD.<br>|
|sar_hud_strafesync_font_index|1|Font index of strafesync HUD.<br>|
|sar_hud_strafesync_offset_x|0|X offset of strafesync HUD.<br>|
|sar_hud_strafesync_offset_y|1000|Y offset of strafesync HUD.<br>|
|sar_hud_strafesync_split_offset_y|1050|Y offset of strafesync HUD.<br>|
|sar_import_stats|cmd|sar_import_stats [filePath]. Import the stats from the specified .csv file.<br>|
|sar_print_stats|cmd|sar_print_stats. Prints your statistics if those are loaded.<br>|
|sar_quickhud_mode|0|Set the quickhud mode :<br>0 : Default quickhud<br>1 : Customizable quickhud<br>2 : quickhud from .png<br>|
|sar_quickhud_set_texture|cmd|sar_quickhud_set_texture \<filepath>. Enter the base name, it will search for \<filepath>1.png, \<filepath>2.png, \<filepath>3.png and \<filepath>4.png<br>ex: sar_quickhud_set_texture "E:\Steam\steamapps\common\Portal 2\portal2\krzyhau"<br>|
|sar_quickhud_size|15|Size of the custom quickhud.<br>|
|sar_quickhud_x|45|Horizontal distance of the custom quickhud.<br>|
|sar_quickhud_y|0|Vertical distance of the custom quickhud.<br>|
|sar_statcounter_filePath|Stats/phunkpaiDWPS.csv|Path to the statcounter .csv file.<br>|
|sar_strafesync|0|Shows strafe sync stats.<br>|
|sar_strafesync_noground|0|0: Always run.<br>1: Do not run when on ground.<br>|
|sar_strafesync_pause|cmd|Pause strafe sync session.<br>|
|sar_strafesync_reset|cmd|Reset strafe sync session.<br>|
|sar_strafesync_resume|cmd|Resume strafe sync session.<br>|
|sar_strafesync_session_time|0|In seconds. How much time should pass until session is reset.<br>If 0, you'll have to reset the session manually.<br>|
|sar_strafesync_split|cmd|Makes a new split.<br>|
