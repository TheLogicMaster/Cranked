// Auto-generated Playdate native function mappings

#include "PlaydateAPI.hpp"
#include "../Emulator.hpp"

NativeFunctionMetadata playdateFunctionTable[FUNCTION_TABLE_SIZE] {
	{ "playdate_sys_realloc", (void *) playdate_sys_realloc, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::ptr_t },
	{ "playdate_sys_formatString", (void *) playdate_sys_formatString, { ArgType::ptr_t, ArgType::ptr_t, ArgType::varargs_t }, ArgType::int32_t },
	{ "playdate_sys_logToConsole", (void *) playdate_sys_logToConsole, { ArgType::ptr_t, ArgType::varargs_t }, ArgType::void_t },
	{ "playdate_sys_error", (void *) playdate_sys_error, { ArgType::ptr_t, ArgType::varargs_t }, ArgType::void_t },
	{ "playdate_sys_getLanguage", (void *) playdate_sys_getLanguage, {  }, ArgType::int32_t },
	{ "playdate_sys_getCurrentTimeMilliseconds", (void *) playdate_sys_getCurrentTimeMilliseconds, {  }, ArgType::uint32_t },
	{ "playdate_sys_getSecondsSinceEpoch", (void *) playdate_sys_getSecondsSinceEpoch, { ArgType::ptr_t }, ArgType::uint32_t },
	{ "playdate_sys_drawFPS", (void *) playdate_sys_drawFPS, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sys_setUpdateCallback", (void *) playdate_sys_setUpdateCallback, { ArgType::uint32_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_getButtonState", (void *) playdate_sys_getButtonState, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_setPeripheralsEnabled", (void *) playdate_sys_setPeripheralsEnabled, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sys_getAccelerometer", (void *) playdate_sys_getAccelerometer, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_getCrankChange", (void *) playdate_sys_getCrankChange, {  }, ArgType::float_t },
	{ "playdate_sys_getCrankAngle", (void *) playdate_sys_getCrankAngle, {  }, ArgType::float_t },
	{ "playdate_sys_isCrankDocked", (void *) playdate_sys_isCrankDocked, {  }, ArgType::int32_t },
	{ "playdate_sys_setCrankSoundsDisabled", (void *) playdate_sys_setCrankSoundsDisabled, { ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_sys_getFlipped", (void *) playdate_sys_getFlipped, {  }, ArgType::int32_t },
	{ "playdate_sys_setAutoLockDisabled", (void *) playdate_sys_setAutoLockDisabled, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sys_setMenuImage", (void *) playdate_sys_setMenuImage, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sys_addMenuItem", (void *) playdate_sys_addMenuItem, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sys_addCheckmarkMenuItem", (void *) playdate_sys_addCheckmarkMenuItem, { ArgType::ptr_t, ArgType::int32_t, ArgType::uint32_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sys_addOptionsMenuItem", (void *) playdate_sys_addOptionsMenuItem, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t, ArgType::uint32_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sys_removeAllMenuItems", (void *) playdate_sys_removeAllMenuItems, {  }, ArgType::void_t },
	{ "playdate_sys_removeMenuItem", (void *) playdate_sys_removeMenuItem, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_getMenuItemValue", (void *) playdate_sys_getMenuItemValue, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sys_setMenuItemValue", (void *) playdate_sys_setMenuItemValue, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sys_getMenuItemTitle", (void *) playdate_sys_getMenuItemTitle, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sys_setMenuItemTitle", (void *) playdate_sys_setMenuItemTitle, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_getMenuItemUserdata", (void *) playdate_sys_getMenuItemUserdata, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sys_setMenuItemUserdata", (void *) playdate_sys_setMenuItemUserdata, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_getReduceFlashing", (void *) playdate_sys_getReduceFlashing, {  }, ArgType::int32_t },
	{ "playdate_sys_getElapsedTime", (void *) playdate_sys_getElapsedTime, {  }, ArgType::float_t },
	{ "playdate_sys_resetElapsedTime", (void *) playdate_sys_resetElapsedTime, {  }, ArgType::void_t },
	{ "playdate_sys_getBatteryPercentage", (void *) playdate_sys_getBatteryPercentage, {  }, ArgType::float_t },
	{ "playdate_sys_getBatteryVoltage", (void *) playdate_sys_getBatteryVoltage, {  }, ArgType::float_t },
	{ "playdate_sys_getTimezoneOffset", (void *) playdate_sys_getTimezoneOffset, {  }, ArgType::int32_t },
	{ "playdate_sys_shouldDisplay24HourTime", (void *) playdate_sys_shouldDisplay24HourTime, {  }, ArgType::int32_t },
	{ "playdate_sys_convertEpochToDateTime", (void *) playdate_sys_convertEpochToDateTime, { ArgType::uint32_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sys_convertDateTimeToEpoch", (void *) playdate_sys_convertDateTimeToEpoch, { ArgType::ptr_t }, ArgType::uint32_t },
	{ "playdate_sys_clearICache", (void *) playdate_sys_clearICache, {  }, ArgType::void_t },
	{ "playdate_file_geterr", (void *) playdate_file_geterr, {  }, ArgType::ptr_t },
	{ "playdate_file_listfiles", (void *) playdate_file_listfiles, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_file_stat", (void *) playdate_file_stat, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_file_mkdir", (void *) playdate_file_mkdir, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_file_unlink", (void *) playdate_file_unlink, { ArgType::ptr_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_file_rename", (void *) playdate_file_rename, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_file_open", (void *) playdate_file_open, { ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_file_close", (void *) playdate_file_close, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_file_read", (void *) playdate_file_read, { ArgType::ptr_t, ArgType::ptr_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_file_write", (void *) playdate_file_write, { ArgType::ptr_t, ArgType::ptr_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_file_flush", (void *) playdate_file_flush, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_file_tell", (void *) playdate_file_tell, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_file_seek", (void *) playdate_file_seek, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_video_loadVideo", (void *) playdate_video_loadVideo, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_video_freePlayer", (void *) playdate_video_freePlayer, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_video_setContext", (void *) playdate_video_setContext, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_video_useScreenContext", (void *) playdate_video_useScreenContext, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_video_renderFrame", (void *) playdate_video_renderFrame, { ArgType::ptr_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_video_getError", (void *) playdate_video_getError, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_video_getInfo", (void *) playdate_video_getInfo, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_video_getContext", (void *) playdate_video_getContext, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_clear", (void *) playdate_graphics_clear, { ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_setBackgroundColor", (void *) playdate_graphics_setBackgroundColor, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_setStencil", (void *) playdate_graphics_setStencil, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_setDrawMode", (void *) playdate_graphics_setDrawMode, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_setDrawOffset", (void *) playdate_graphics_setDrawOffset, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_setClipRect", (void *) playdate_graphics_setClipRect, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_clearClipRect", (void *) playdate_graphics_clearClipRect, {  }, ArgType::void_t },
	{ "playdate_graphics_setLineCapStyle", (void *) playdate_graphics_setLineCapStyle, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_setFont", (void *) playdate_graphics_setFont, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_setTextTracking", (void *) playdate_graphics_setTextTracking, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_pushContext", (void *) playdate_graphics_pushContext, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_popContext", (void *) playdate_graphics_popContext, {  }, ArgType::void_t },
	{ "playdate_graphics_drawBitmap", (void *) playdate_graphics_drawBitmap, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_tileBitmap", (void *) playdate_graphics_tileBitmap, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_drawLine", (void *) playdate_graphics_drawLine, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_fillTriangle", (void *) playdate_graphics_fillTriangle, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_drawRect", (void *) playdate_graphics_drawRect, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_fillRect", (void *) playdate_graphics_fillRect, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_drawEllipse", (void *) playdate_graphics_drawEllipse, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::float_t, ArgType::float_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_fillEllipse", (void *) playdate_graphics_fillEllipse, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::float_t, ArgType::float_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_drawScaledBitmap", (void *) playdate_graphics_drawScaledBitmap, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_graphics_drawText", (void *) playdate_graphics_drawText, { ArgType::ptr_t, ArgType::uint32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_graphics_newBitmap", (void *) playdate_graphics_newBitmap, { ArgType::int32_t, ArgType::int32_t, ArgType::uint32_t }, ArgType::ptr_t },
	{ "playdate_graphics_freeBitmap", (void *) playdate_graphics_freeBitmap, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_loadBitmap", (void *) playdate_graphics_loadBitmap, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_copyBitmap", (void *) playdate_graphics_copyBitmap, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_loadIntoBitmap", (void *) playdate_graphics_loadIntoBitmap, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_getBitmapData", (void *) playdate_graphics_getBitmapData, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_clearBitmap", (void *) playdate_graphics_clearBitmap, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_graphics_rotatedBitmap", (void *) playdate_graphics_rotatedBitmap, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_newBitmapTable", (void *) playdate_graphics_newBitmapTable, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_graphics_freeBitmapTable", (void *) playdate_graphics_freeBitmapTable, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_loadBitmapTable", (void *) playdate_graphics_loadBitmapTable, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_loadIntoBitmapTable", (void *) playdate_graphics_loadIntoBitmapTable, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_graphics_getTableBitmap", (void *) playdate_graphics_getTableBitmap, { ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_graphics_loadFont", (void *) playdate_graphics_loadFont, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_getFontPage", (void *) playdate_graphics_getFontPage, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::ptr_t },
	{ "playdate_graphics_getPageGlyph", (void *) playdate_graphics_getPageGlyph, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_getGlyphKerning", (void *) playdate_graphics_getGlyphKerning, { ArgType::ptr_t, ArgType::uint32_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_graphics_getTextWidth", (void *) playdate_graphics_getTextWidth, { ArgType::ptr_t, ArgType::ptr_t, ArgType::uint32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_graphics_getFrame", (void *) playdate_graphics_getFrame, {  }, ArgType::ptr_t },
	{ "playdate_graphics_getDisplayFrame", (void *) playdate_graphics_getDisplayFrame, {  }, ArgType::ptr_t },
	{ "playdate_graphics_copyFrameBufferBitmap", (void *) playdate_graphics_copyFrameBufferBitmap, {  }, ArgType::ptr_t },
	{ "playdate_graphics_markUpdatedRows", (void *) playdate_graphics_markUpdatedRows, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_display", (void *) playdate_graphics_display, {  }, ArgType::void_t },
	{ "playdate_graphics_setColorToPattern", (void *) playdate_graphics_setColorToPattern, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_checkMaskCollision", (void *) playdate_graphics_checkMaskCollision, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::struct4_t }, ArgType::int32_t },
	{ "playdate_graphics_setScreenClipRect", (void *) playdate_graphics_setScreenClipRect, { ArgType::int32_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_fillPolygon", (void *) playdate_graphics_fillPolygon, { ArgType::int32_t, ArgType::ptr_t, ArgType::uint32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_getFontHeight", (void *) playdate_graphics_getFontHeight, { ArgType::ptr_t }, ArgType::uint8_t },
	{ "playdate_graphics_getDisplayBufferBitmap", (void *) playdate_graphics_getDisplayBufferBitmap, {  }, ArgType::ptr_t },
	{ "playdate_graphics_drawRotatedBitmap", (void *) playdate_graphics_drawRotatedBitmap, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_graphics_setTextLeading", (void *) playdate_graphics_setTextLeading, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_setBitmapMask", (void *) playdate_graphics_setBitmapMask, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_graphics_getBitmapMask", (void *) playdate_graphics_getBitmapMask, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_graphics_setStencilImage", (void *) playdate_graphics_setStencilImage, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_graphics_makeFontFromData", (void *) playdate_graphics_makeFontFromData, { ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sprite_setAlwaysRedraw", (void *) playdate_sprite_setAlwaysRedraw, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_addDirtyRect", (void *) playdate_sprite_addDirtyRect, { ArgType::struct4_t }, ArgType::void_t },
	{ "playdate_sprite_drawSprites", (void *) playdate_sprite_drawSprites, {  }, ArgType::void_t },
	{ "playdate_sprite_updateAndDrawSprites", (void *) playdate_sprite_updateAndDrawSprites, {  }, ArgType::void_t },
	{ "playdate_sprite_newSprite", (void *) playdate_sprite_newSprite, {  }, ArgType::ptr_t },
	{ "playdate_sprite_freeSprite", (void *) playdate_sprite_freeSprite, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_copy", (void *) playdate_sprite_copy, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_addSprite", (void *) playdate_sprite_addSprite, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_removeSprite", (void *) playdate_sprite_removeSprite, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_removeSprites", (void *) playdate_sprite_removeSprites, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_removeAllSprites", (void *) playdate_sprite_removeAllSprites, {  }, ArgType::void_t },
	{ "playdate_sprite_getSpriteCount", (void *) playdate_sprite_getSpriteCount, {  }, ArgType::int32_t },
	{ "playdate_sprite_setBounds", (void *) playdate_sprite_setBounds, { ArgType::ptr_t, ArgType::struct4_t }, ArgType::void_t },
	{ "playdate_sprite_getBounds", (void *) playdate_sprite_getBounds, { ArgType::ptr_t }, ArgType::struct4_t },
	{ "playdate_sprite_moveTo", (void *) playdate_sprite_moveTo, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sprite_moveBy", (void *) playdate_sprite_moveBy, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sprite_setImage", (void *) playdate_sprite_setImage, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_getImage", (void *) playdate_sprite_getImage, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_setSize", (void *) playdate_sprite_setSize, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sprite_setZIndex", (void *) playdate_sprite_setZIndex, { ArgType::ptr_t, ArgType::int16_t }, ArgType::void_t },
	{ "playdate_sprite_getZIndex", (void *) playdate_sprite_getZIndex, { ArgType::ptr_t }, ArgType::int16_t },
	{ "playdate_sprite_setDrawMode", (void *) playdate_sprite_setDrawMode, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_setImageFlip", (void *) playdate_sprite_setImageFlip, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_getImageFlip", (void *) playdate_sprite_getImageFlip, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sprite_setStencil", (void *) playdate_sprite_setStencil, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_setClipRect", (void *) playdate_sprite_setClipRect, { ArgType::ptr_t, ArgType::struct4_t }, ArgType::void_t },
	{ "playdate_sprite_clearClipRect", (void *) playdate_sprite_clearClipRect, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_setClipRectsInRange", (void *) playdate_sprite_setClipRectsInRange, { ArgType::struct4_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_clearClipRectsInRange", (void *) playdate_sprite_clearClipRectsInRange, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_setUpdatesEnabled", (void *) playdate_sprite_setUpdatesEnabled, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_updatesEnabled", (void *) playdate_sprite_updatesEnabled, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sprite_setCollisionsEnabled", (void *) playdate_sprite_setCollisionsEnabled, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_collisionsEnabled", (void *) playdate_sprite_collisionsEnabled, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sprite_setVisible", (void *) playdate_sprite_setVisible, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_isVisible", (void *) playdate_sprite_isVisible, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sprite_setOpaque", (void *) playdate_sprite_setOpaque, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_markDirty", (void *) playdate_sprite_markDirty, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_setTag", (void *) playdate_sprite_setTag, { ArgType::ptr_t, ArgType::uint8_t }, ArgType::void_t },
	{ "playdate_sprite_getTag", (void *) playdate_sprite_getTag, { ArgType::ptr_t }, ArgType::uint8_t },
	{ "playdate_sprite_setIgnoresDrawOffset", (void *) playdate_sprite_setIgnoresDrawOffset, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sprite_setUpdateFunction", (void *) playdate_sprite_setUpdateFunction, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sprite_setDrawFunction", (void *) playdate_sprite_setDrawFunction, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sprite_getPosition", (void *) playdate_sprite_getPosition, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_resetCollisionWorld", (void *) playdate_sprite_resetCollisionWorld, {  }, ArgType::void_t },
	{ "playdate_sprite_setCollideRect", (void *) playdate_sprite_setCollideRect, { ArgType::ptr_t, ArgType::struct4_t }, ArgType::void_t },
	{ "playdate_sprite_getCollideRect", (void *) playdate_sprite_getCollideRect, { ArgType::ptr_t }, ArgType::struct4_t },
	{ "playdate_sprite_clearCollideRect", (void *) playdate_sprite_clearCollideRect, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_setCollisionResponseFunction", (void *) playdate_sprite_setCollisionResponseFunction, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sprite_checkCollisions", (void *) playdate_sprite_checkCollisions, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_moveWithCollisions", (void *) playdate_sprite_moveWithCollisions, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_querySpritesAtPoint", (void *) playdate_sprite_querySpritesAtPoint, { ArgType::float_t, ArgType::float_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_querySpritesInRect", (void *) playdate_sprite_querySpritesInRect, { ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_querySpritesAlongLine", (void *) playdate_sprite_querySpritesAlongLine, { ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_querySpriteInfoAlongLine", (void *) playdate_sprite_querySpriteInfoAlongLine, { ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_overlappingSprites", (void *) playdate_sprite_overlappingSprites, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_allOverlappingSprites", (void *) playdate_sprite_allOverlappingSprites, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_setStencilPattern", (void *) playdate_sprite_setStencilPattern, { ArgType::ptr_t, ArgType::uint8_t }, ArgType::void_t },
	{ "playdate_sprite_clearStencil", (void *) playdate_sprite_clearStencil, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_setUserdata", (void *) playdate_sprite_setUserdata, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sprite_getUserdata", (void *) playdate_sprite_getUserdata, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sprite_setStencilImage", (void *) playdate_sprite_setStencilImage, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_display_getWidth", (void *) playdate_display_getWidth, {  }, ArgType::int32_t },
	{ "playdate_display_getHeight", (void *) playdate_display_getHeight, {  }, ArgType::int32_t },
	{ "playdate_display_setRefreshRate", (void *) playdate_display_setRefreshRate, { ArgType::float_t }, ArgType::void_t },
	{ "playdate_display_setInverted", (void *) playdate_display_setInverted, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_display_setScale", (void *) playdate_display_setScale, { ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_display_setMosaic", (void *) playdate_display_setMosaic, { ArgType::uint32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_display_setFlipped", (void *) playdate_display_setFlipped, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_display_setOffset", (void *) playdate_display_setOffset, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_channel_newChannel", (void *) playdate_sound_channel_newChannel, {  }, ArgType::ptr_t },
	{ "playdate_sound_channel_freeChannel", (void *) playdate_sound_channel_freeChannel, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_channel_addSource", (void *) playdate_sound_channel_addSource, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_channel_removeSource", (void *) playdate_sound_channel_removeSource, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_channel_addCallbackSource", (void *) playdate_sound_channel_addCallbackSource, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_channel_addEffect", (void *) playdate_sound_channel_addEffect, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_channel_removeEffect", (void *) playdate_sound_channel_removeEffect, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_channel_setVolume", (void *) playdate_sound_channel_setVolume, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_channel_getVolume", (void *) playdate_sound_channel_getVolume, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_channel_setVolumeModulator", (void *) playdate_sound_channel_setVolumeModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_channel_getVolumeModulator", (void *) playdate_sound_channel_getVolumeModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_channel_setPan", (void *) playdate_sound_channel_setPan, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_channel_setPanModulator", (void *) playdate_sound_channel_setPanModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_channel_getPanModulator", (void *) playdate_sound_channel_getPanModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_channel_getDryLevelSignal", (void *) playdate_sound_channel_getDryLevelSignal, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_channel_getWetLevelSignal", (void *) playdate_sound_channel_getWetLevelSignal, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_fileplayer_newPlayer", (void *) playdate_sound_fileplayer_newPlayer, {  }, ArgType::ptr_t },
	{ "playdate_sound_fileplayer_freePlayer", (void *) playdate_sound_fileplayer_freePlayer, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_loadIntoPlayer", (void *) playdate_sound_fileplayer_loadIntoPlayer, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_fileplayer_setBufferLength", (void *) playdate_sound_fileplayer_setBufferLength, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_play", (void *) playdate_sound_fileplayer_play, { ArgType::ptr_t, ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_sound_fileplayer_isPlaying", (void *) playdate_sound_fileplayer_isPlaying, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_fileplayer_pause", (void *) playdate_sound_fileplayer_pause, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_stop", (void *) playdate_sound_fileplayer_stop, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_setVolume", (void *) playdate_sound_fileplayer_setVolume, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_getVolume", (void *) playdate_sound_fileplayer_getVolume, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_getLength", (void *) playdate_sound_fileplayer_getLength, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_fileplayer_setOffset", (void *) playdate_sound_fileplayer_setOffset, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_setRate", (void *) playdate_sound_fileplayer_setRate, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_setLoopRange", (void *) playdate_sound_fileplayer_setLoopRange, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_didUnderrun", (void *) playdate_sound_fileplayer_didUnderrun, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_fileplayer_setFinishCallback", (void *) playdate_sound_fileplayer_setFinishCallback, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_setLoopCallback", (void *) playdate_sound_fileplayer_setLoopCallback, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_getOffset", (void *) playdate_sound_fileplayer_getOffset, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_fileplayer_getRate", (void *) playdate_sound_fileplayer_getRate, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_fileplayer_setStopOnUnderrun", (void *) playdate_sound_fileplayer_setStopOnUnderrun, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_fadeVolume", (void *) playdate_sound_fileplayer_fadeVolume, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::int32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_fileplayer_setMP3StreamSource", (void *) playdate_sound_fileplayer_setMP3StreamSource, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_sample_newSampleBuffer", (void *) playdate_sound_sample_newSampleBuffer, { ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_sample_loadIntoSample", (void *) playdate_sound_sample_loadIntoSample, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sample_load", (void *) playdate_sound_sample_load, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_sample_newSampleFromData", (void *) playdate_sound_sample_newSampleFromData, { ArgType::ptr_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_sample_getData", (void *) playdate_sound_sample_getData, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sample_freeSample", (void *) playdate_sound_sample_freeSample, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sample_getLength", (void *) playdate_sound_sample_getLength, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_sampleplayer_newPlayer", (void *) playdate_sound_sampleplayer_newPlayer, {  }, ArgType::ptr_t },
	{ "playdate_sound_sampleplayer_freePlayer", (void *) playdate_sound_sampleplayer_freePlayer, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_setSample", (void *) playdate_sound_sampleplayer_setSample, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_play", (void *) playdate_sound_sampleplayer_play, { ArgType::ptr_t, ArgType::int32_t, ArgType::float_t }, ArgType::int32_t },
	{ "playdate_sound_sampleplayer_isPlaying", (void *) playdate_sound_sampleplayer_isPlaying, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sampleplayer_stop", (void *) playdate_sound_sampleplayer_stop, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_setVolume", (void *) playdate_sound_sampleplayer_setVolume, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_getVolume", (void *) playdate_sound_sampleplayer_getVolume, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_getLength", (void *) playdate_sound_sampleplayer_getLength, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_sampleplayer_setOffset", (void *) playdate_sound_sampleplayer_setOffset, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_setRate", (void *) playdate_sound_sampleplayer_setRate, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_setPlayRange", (void *) playdate_sound_sampleplayer_setPlayRange, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_setFinishCallback", (void *) playdate_sound_sampleplayer_setFinishCallback, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_setLoopCallback", (void *) playdate_sound_sampleplayer_setLoopCallback, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_sampleplayer_getOffset", (void *) playdate_sound_sampleplayer_getOffset, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_sampleplayer_getRate", (void *) playdate_sound_sampleplayer_getRate, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_sampleplayer_setPaused", (void *) playdate_sound_sampleplayer_setPaused, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_synth_newSynth", (void *) playdate_sound_synth_newSynth, {  }, ArgType::ptr_t },
	{ "playdate_sound_synth_freeSynth", (void *) playdate_sound_synth_freeSynth, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_setWaveform", (void *) playdate_sound_synth_setWaveform, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_synth_setGenerator", (void *) playdate_sound_synth_setGenerator, { ArgType::ptr_t, ArgType::int32_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_setSample", (void *) playdate_sound_synth_setSample, { ArgType::ptr_t, ArgType::ptr_t, ArgType::uint32_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_synth_setAttackTime", (void *) playdate_sound_synth_setAttackTime, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_synth_setDecayTime", (void *) playdate_sound_synth_setDecayTime, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_synth_setSustainLevel", (void *) playdate_sound_synth_setSustainLevel, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_synth_setReleaseTime", (void *) playdate_sound_synth_setReleaseTime, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_synth_setTranspose", (void *) playdate_sound_synth_setTranspose, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_synth_setFrequencyModulator", (void *) playdate_sound_synth_setFrequencyModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_getFrequencyModulator", (void *) playdate_sound_synth_getFrequencyModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_synth_setAmplitudeModulator", (void *) playdate_sound_synth_setAmplitudeModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_getAmplitudeModulator", (void *) playdate_sound_synth_getAmplitudeModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_synth_getParameterCount", (void *) playdate_sound_synth_getParameterCount, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_synth_setParameter", (void *) playdate_sound_synth_setParameter, { ArgType::ptr_t, ArgType::int32_t, ArgType::float_t }, ArgType::int32_t },
	{ "playdate_sound_synth_setParameterModulator", (void *) playdate_sound_synth_setParameterModulator, { ArgType::ptr_t, ArgType::int32_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_getParameterModulator", (void *) playdate_sound_synth_getParameterModulator, { ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_synth_playNote", (void *) playdate_sound_synth_playNote, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_synth_playMIDINote", (void *) playdate_sound_synth_playMIDINote, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_synth_noteOff", (void *) playdate_sound_synth_noteOff, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_synth_stop", (void *) playdate_sound_synth_stop, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_setVolume", (void *) playdate_sound_synth_setVolume, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_synth_getVolume", (void *) playdate_sound_synth_getVolume, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_synth_isPlaying", (void *) playdate_sound_synth_isPlaying, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_synth_getEnvelope", (void *) playdate_sound_synth_getEnvelope, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_sequence_newSequence", (void *) playdate_sound_sequence_newSequence, {  }, ArgType::ptr_t },
	{ "playdate_sound_sequence_freeSequence", (void *) playdate_sound_sequence_freeSequence, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sequence_loadMidiFile", (void *) playdate_sound_sequence_loadMidiFile, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sequence_getTime", (void *) playdate_sound_sequence_getTime, { ArgType::ptr_t }, ArgType::uint32_t },
	{ "playdate_sound_sequence_setTime", (void *) playdate_sound_sequence_setTime, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_sequence_setLoops", (void *) playdate_sound_sequence_setLoops, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_sequence_getTempo", (void *) playdate_sound_sequence_getTempo, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sequence_setTempo", (void *) playdate_sound_sequence_setTempo, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_sequence_getTrackCount", (void *) playdate_sound_sequence_getTrackCount, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sequence_addTrack", (void *) playdate_sound_sequence_addTrack, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_sequence_getTrackAtIndex", (void *) playdate_sound_sequence_getTrackAtIndex, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::ptr_t },
	{ "playdate_sound_sequence_setTrackAtIndex", (void *) playdate_sound_sequence_setTrackAtIndex, { ArgType::ptr_t, ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_sequence_allNotesOff", (void *) playdate_sound_sequence_allNotesOff, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sequence_isPlaying", (void *) playdate_sound_sequence_isPlaying, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sequence_getLength", (void *) playdate_sound_sequence_getLength, { ArgType::ptr_t }, ArgType::uint32_t },
	{ "playdate_sound_sequence_play", (void *) playdate_sound_sequence_play, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sequence_stop", (void *) playdate_sound_sequence_stop, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_sequence_getCurrentStep", (void *) playdate_sound_sequence_getCurrentStep, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_sequence_setCurrentStep", (void *) playdate_sound_sequence_setCurrentStep, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_effect_newEffect", (void *) playdate_sound_effect_newEffect, { ArgType::uint32_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_freeEffect", (void *) playdate_sound_effect_freeEffect, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_setMix", (void *) playdate_sound_effect_setMix, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_setMixModulator", (void *) playdate_sound_effect_setMixModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_getMixModulator", (void *) playdate_sound_effect_getMixModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_setUserdata", (void *) playdate_sound_effect_setUserdata, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_getUserdata", (void *) playdate_sound_effect_getUserdata, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_twopolefilter_newFilter", (void *) playdate_sound_effect_twopolefilter_newFilter, {  }, ArgType::ptr_t },
	{ "playdate_sound_effect_twopolefilter_freeFilter", (void *) playdate_sound_effect_twopolefilter_freeFilter, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_setType", (void *) playdate_sound_effect_twopolefilter_setType, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_setFrequency", (void *) playdate_sound_effect_twopolefilter_setFrequency, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_setFrequencyModulator", (void *) playdate_sound_effect_twopolefilter_setFrequencyModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_getFrequencyModulator", (void *) playdate_sound_effect_twopolefilter_getFrequencyModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_twopolefilter_setGain", (void *) playdate_sound_effect_twopolefilter_setGain, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_setResonance", (void *) playdate_sound_effect_twopolefilter_setResonance, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_setResonanceModulator", (void *) playdate_sound_effect_twopolefilter_setResonanceModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_twopolefilter_getResonanceModulator", (void *) playdate_sound_effect_twopolefilter_getResonanceModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_onepolefilter_newFilter", (void *) playdate_sound_effect_onepolefilter_newFilter, {  }, ArgType::ptr_t },
	{ "playdate_sound_effect_onepolefilter_freeFilter", (void *) playdate_sound_effect_onepolefilter_freeFilter, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_onepolefilter_setParameter", (void *) playdate_sound_effect_onepolefilter_setParameter, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_onepolefilter_setParameterModulator", (void *) playdate_sound_effect_onepolefilter_setParameterModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_onepolefilter_getParameterModulator", (void *) playdate_sound_effect_onepolefilter_getParameterModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_bitcrusher_newBitCrusher", (void *) playdate_sound_effect_bitcrusher_newBitCrusher, {  }, ArgType::ptr_t },
	{ "playdate_sound_effect_bitcrusher_freeBitCrusher", (void *) playdate_sound_effect_bitcrusher_freeBitCrusher, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_bitcrusher_setAmount", (void *) playdate_sound_effect_bitcrusher_setAmount, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_bitcrusher_setAmountModulator", (void *) playdate_sound_effect_bitcrusher_setAmountModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_bitcrusher_getAmountModulator", (void *) playdate_sound_effect_bitcrusher_getAmountModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_bitcrusher_setUndersampling", (void *) playdate_sound_effect_bitcrusher_setUndersampling, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_bitcrusher_setUndersampleModulator", (void *) playdate_sound_effect_bitcrusher_setUndersampleModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_bitcrusher_getUndersampleModulator", (void *) playdate_sound_effect_bitcrusher_getUndersampleModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_ringmodulator_newRingmod", (void *) playdate_sound_effect_ringmodulator_newRingmod, {  }, ArgType::ptr_t },
	{ "playdate_sound_effect_ringmodulator_freeRingmod", (void *) playdate_sound_effect_ringmodulator_freeRingmod, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_ringmodulator_setFrequency", (void *) playdate_sound_effect_ringmodulator_setFrequency, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_ringmodulator_setFrequencyModulator", (void *) playdate_sound_effect_ringmodulator_setFrequencyModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_ringmodulator_getFrequencyModulator", (void *) playdate_sound_effect_ringmodulator_getFrequencyModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_delayline_newDelayLine", (void *) playdate_sound_effect_delayline_newDelayLine, { ArgType::int32_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_delayline_freeDelayLine", (void *) playdate_sound_effect_delayline_freeDelayLine, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_delayline_setLength", (void *) playdate_sound_effect_delayline_setLength, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_effect_delayline_setFeedback", (void *) playdate_sound_effect_delayline_setFeedback, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_delayline_addTap", (void *) playdate_sound_effect_delayline_addTap, { ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_delayline_freeTap", (void *) playdate_sound_effect_delayline_freeTap, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_delayline_setTapDelay", (void *) playdate_sound_effect_delayline_setTapDelay, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_effect_delayline_setTapDelayModulator", (void *) playdate_sound_effect_delayline_setTapDelayModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_delayline_getTapDelayModulator", (void *) playdate_sound_effect_delayline_getTapDelayModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_delayline_setTapChannelsFlipped", (void *) playdate_sound_effect_delayline_setTapChannelsFlipped, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_newOverdrive", (void *) playdate_sound_effect_overdrive_newOverdrive, {  }, ArgType::ptr_t },
	{ "playdate_sound_effect_overdrive_freeOverdrive", (void *) playdate_sound_effect_overdrive_freeOverdrive, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_setGain", (void *) playdate_sound_effect_overdrive_setGain, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_setLimit", (void *) playdate_sound_effect_overdrive_setLimit, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_setLimitModulator", (void *) playdate_sound_effect_overdrive_setLimitModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_getLimitModulator", (void *) playdate_sound_effect_overdrive_getLimitModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_effect_overdrive_setOffset", (void *) playdate_sound_effect_overdrive_setOffset, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_setOffsetModulator", (void *) playdate_sound_effect_overdrive_setOffsetModulator, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_effect_overdrive_getOffsetModulator", (void *) playdate_sound_effect_overdrive_getOffsetModulator, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_lfo_newLFO", (void *) playdate_sound_lfo_newLFO, { ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_lfo_freeLFO", (void *) playdate_sound_lfo_freeLFO, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setType", (void *) playdate_sound_lfo_setType, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setRate", (void *) playdate_sound_lfo_setRate, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setPhase", (void *) playdate_sound_lfo_setPhase, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setCenter", (void *) playdate_sound_lfo_setCenter, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setDepth", (void *) playdate_sound_lfo_setDepth, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setArpeggiation", (void *) playdate_sound_lfo_setArpeggiation, { ArgType::ptr_t, ArgType::int32_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setFunction", (void *) playdate_sound_lfo_setFunction, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setDelay", (void *) playdate_sound_lfo_setDelay, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_lfo_setRetrigger", (void *) playdate_sound_lfo_setRetrigger, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_lfo_getValue", (void *) playdate_sound_lfo_getValue, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_lfo_setGlobal", (void *) playdate_sound_lfo_setGlobal, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_envelope_newEnvelope", (void *) playdate_sound_envelope_newEnvelope, { ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::float_t }, ArgType::ptr_t },
	{ "playdate_sound_envelope_freeEnvelope", (void *) playdate_sound_envelope_freeEnvelope, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setAttack", (void *) playdate_sound_envelope_setAttack, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setDecay", (void *) playdate_sound_envelope_setDecay, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setSustain", (void *) playdate_sound_envelope_setSustain, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setRelease", (void *) playdate_sound_envelope_setRelease, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setLegato", (void *) playdate_sound_envelope_setLegato, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setRetrigger", (void *) playdate_sound_envelope_setRetrigger, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_envelope_getValue", (void *) playdate_sound_envelope_getValue, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_envelope_setCurvature", (void *) playdate_sound_envelope_setCurvature, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setVelocitySensitivity", (void *) playdate_sound_envelope_setVelocitySensitivity, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_envelope_setRateScaling", (void *) playdate_sound_envelope_setRateScaling, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_source_setVolume", (void *) playdate_sound_source_setVolume, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_source_getVolume", (void *) playdate_sound_source_getVolume, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_source_isPlaying", (void *) playdate_sound_source_isPlaying, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_source_setFinishCallback", (void *) playdate_sound_source_setFinishCallback, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_control_signal_newSignal", (void *) playdate_control_signal_newSignal, {  }, ArgType::ptr_t },
	{ "playdate_control_signal_freeSignal", (void *) playdate_control_signal_freeSignal, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_control_signal_clearEvents", (void *) playdate_control_signal_clearEvents, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_control_signal_addEvent", (void *) playdate_control_signal_addEvent, { ArgType::ptr_t, ArgType::int32_t, ArgType::float_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_control_signal_removeEvent", (void *) playdate_control_signal_removeEvent, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_control_signal_getMIDIControllerNumber", (void *) playdate_control_signal_getMIDIControllerNumber, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_track_newTrack", (void *) playdate_sound_track_newTrack, {  }, ArgType::ptr_t },
	{ "playdate_sound_track_freeTrack", (void *) playdate_sound_track_freeTrack, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_track_setInstrument", (void *) playdate_sound_track_setInstrument, { ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_track_getInstrument", (void *) playdate_sound_track_getInstrument, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_track_addNoteEvent", (void *) playdate_sound_track_addNoteEvent, { ArgType::ptr_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_track_removeNoteEvent", (void *) playdate_sound_track_removeNoteEvent, { ArgType::ptr_t, ArgType::uint32_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_track_clearNotes", (void *) playdate_sound_track_clearNotes, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_track_getControlSignalCount", (void *) playdate_sound_track_getControlSignalCount, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_track_getControlSignal", (void *) playdate_sound_track_getControlSignal, { ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_track_clearControlEvents", (void *) playdate_sound_track_clearControlEvents, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_track_getPolyphony", (void *) playdate_sound_track_getPolyphony, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_track_activeVoiceCount", (void *) playdate_sound_track_activeVoiceCount, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_track_setMuted", (void *) playdate_sound_track_setMuted, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_track_getLength", (void *) playdate_sound_track_getLength, { ArgType::ptr_t }, ArgType::uint32_t },
	{ "playdate_sound_track_getIndexForStep", (void *) playdate_sound_track_getIndexForStep, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_sound_track_getNoteAtIndex", (void *) playdate_sound_track_getNoteAtIndex, { ArgType::ptr_t, ArgType::int32_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_track_getSignalForController", (void *) playdate_sound_track_getSignalForController, { ArgType::ptr_t, ArgType::int32_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_instrument_newInstrument", (void *) playdate_sound_instrument_newInstrument, {  }, ArgType::ptr_t },
	{ "playdate_sound_instrument_freeInstrument", (void *) playdate_sound_instrument_freeInstrument, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_instrument_addVoice", (void *) playdate_sound_instrument_addVoice, { ArgType::ptr_t, ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t }, ArgType::int32_t },
	{ "playdate_sound_instrument_playNote", (void *) playdate_sound_instrument_playNote, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::uint32_t }, ArgType::ptr_t },
	{ "playdate_sound_instrument_playMIDINote", (void *) playdate_sound_instrument_playMIDINote, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t, ArgType::float_t, ArgType::uint32_t }, ArgType::ptr_t },
	{ "playdate_sound_instrument_setPitchBend", (void *) playdate_sound_instrument_setPitchBend, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_instrument_setPitchBendRange", (void *) playdate_sound_instrument_setPitchBendRange, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_instrument_setTranspose", (void *) playdate_sound_instrument_setTranspose, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_instrument_noteOff", (void *) playdate_sound_instrument_noteOff, { ArgType::ptr_t, ArgType::float_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_instrument_allNotesOff", (void *) playdate_sound_instrument_allNotesOff, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_instrument_setVolume", (void *) playdate_sound_instrument_setVolume, { ArgType::ptr_t, ArgType::float_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_instrument_getVolume", (void *) playdate_sound_instrument_getVolume, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_instrument_activeVoiceCount", (void *) playdate_sound_instrument_activeVoiceCount, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_getCurrentTime", (void *) playdate_sound_getCurrentTime, {  }, ArgType::uint32_t },
	{ "playdate_sound_addSource", (void *) playdate_sound_addSource, { ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_sound_getDefaultChannel", (void *) playdate_sound_getDefaultChannel, {  }, ArgType::ptr_t },
	{ "playdate_sound_addChannel", (void *) playdate_sound_addChannel, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_removeChannel", (void *) playdate_sound_removeChannel, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_setMicCallback", (void *) playdate_sound_setMicCallback, { ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_getHeadphoneState", (void *) playdate_sound_getHeadphoneState, { ArgType::ptr_t, ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_sound_setOutputsActive", (void *) playdate_sound_setOutputsActive, { ArgType::int32_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_sound_removeSource", (void *) playdate_sound_removeSource, { ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_sound_signal_newSignal", (void *) playdate_sound_signal_newSignal, { ArgType::uint32_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::uint32_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_sound_signal_freeSignal", (void *) playdate_sound_signal_freeSignal, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_sound_signal_getValue", (void *) playdate_sound_signal_getValue, { ArgType::ptr_t }, ArgType::float_t },
	{ "playdate_sound_signal_setValueScale", (void *) playdate_sound_signal_setValueScale, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_sound_signal_setValueOffset", (void *) playdate_sound_signal_setValueOffset, { ArgType::ptr_t, ArgType::float_t }, ArgType::void_t },
	{ "playdate_lua_addFunction", (void *) playdate_lua_addFunction, { ArgType::uint32_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_lua_registerClass", (void *) playdate_lua_registerClass, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_lua_pushFunction", (void *) playdate_lua_pushFunction, { ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_lua_indexMetatable", (void *) playdate_lua_indexMetatable, {  }, ArgType::int32_t },
	{ "playdate_lua_stop", (void *) playdate_lua_stop, {  }, ArgType::void_t },
	{ "playdate_lua_start", (void *) playdate_lua_start, {  }, ArgType::void_t },
	{ "playdate_lua_getArgCount", (void *) playdate_lua_getArgCount, {  }, ArgType::int32_t },
	{ "playdate_lua_getArgType", (void *) playdate_lua_getArgType, { ArgType::int32_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_lua_argIsNil", (void *) playdate_lua_argIsNil, { ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_lua_getArgBool", (void *) playdate_lua_getArgBool, { ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_lua_getArgInt", (void *) playdate_lua_getArgInt, { ArgType::int32_t }, ArgType::int32_t },
	{ "playdate_lua_getArgFloat", (void *) playdate_lua_getArgFloat, { ArgType::int32_t }, ArgType::float_t },
	{ "playdate_lua_getArgString", (void *) playdate_lua_getArgString, { ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_lua_getArgBytes", (void *) playdate_lua_getArgBytes, { ArgType::int32_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_lua_getArgObject", (void *) playdate_lua_getArgObject, { ArgType::int32_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_lua_getBitmap", (void *) playdate_lua_getBitmap, { ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_lua_getSprite", (void *) playdate_lua_getSprite, { ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_lua_pushNil", (void *) playdate_lua_pushNil, {  }, ArgType::void_t },
	{ "playdate_lua_pushBool", (void *) playdate_lua_pushBool, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_lua_pushInt", (void *) playdate_lua_pushInt, { ArgType::int32_t }, ArgType::void_t },
	{ "playdate_lua_pushFloat", (void *) playdate_lua_pushFloat, { ArgType::float_t }, ArgType::void_t },
	{ "playdate_lua_pushString", (void *) playdate_lua_pushString, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_lua_pushBytes", (void *) playdate_lua_pushBytes, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_lua_pushBitmap", (void *) playdate_lua_pushBitmap, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_lua_pushSprite", (void *) playdate_lua_pushSprite, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_lua_pushObject", (void *) playdate_lua_pushObject, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::ptr_t },
	{ "playdate_lua_retainObject", (void *) playdate_lua_retainObject, { ArgType::ptr_t }, ArgType::ptr_t },
	{ "playdate_lua_releaseObject", (void *) playdate_lua_releaseObject, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_lua_setUserValue", (void *) playdate_lua_setUserValue, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::void_t },
	{ "playdate_lua_getUserValue", (void *) playdate_lua_getUserValue, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_lua_callFunction_deprecated", (void *) playdate_lua_callFunction_deprecated, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_lua_callFunction", (void *) playdate_lua_callFunction, { ArgType::ptr_t, ArgType::int32_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_json_initEncoder", (void *) playdate_json_initEncoder, { ArgType::ptr_t, ArgType::uint32_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "playdate_json_decode", (void *) playdate_json_decode, { ArgType::ptr_t, ArgType::struct2_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_json_decodeString", (void *) playdate_json_decodeString, { ArgType::ptr_t, ArgType::ptr_t, ArgType::ptr_t }, ArgType::int32_t },
	{ "playdate_scoreboards_addScore", (void *) playdate_scoreboards_addScore, { ArgType::ptr_t, ArgType::uint32_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_scoreboards_getPersonalBest", (void *) playdate_scoreboards_getPersonalBest, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_scoreboards_freeScore", (void *) playdate_scoreboards_freeScore, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_scoreboards_getScoreboards", (void *) playdate_scoreboards_getScoreboards, { ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_scoreboards_freeBoardsList", (void *) playdate_scoreboards_freeBoardsList, { ArgType::ptr_t }, ArgType::void_t },
	{ "playdate_scoreboards_getScores", (void *) playdate_scoreboards_getScores, { ArgType::ptr_t, ArgType::uint32_t }, ArgType::int32_t },
	{ "playdate_scoreboards_freeScoresList", (void *) playdate_scoreboards_freeScoresList, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_startArray", (void *) json_encoder_startArray, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_addArrayMember", (void *) json_encoder_addArrayMember, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_endArray", (void *) json_encoder_endArray, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_startTable", (void *) json_encoder_startTable, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_addTableMember", (void *) json_encoder_addTableMember, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "json_encoder_endTable", (void *) json_encoder_endTable, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_writeNull", (void *) json_encoder_writeNull, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_writeFalse", (void *) json_encoder_writeFalse, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_writeTrue", (void *) json_encoder_writeTrue, { ArgType::ptr_t }, ArgType::void_t },
	{ "json_encoder_writeInt", (void *) json_encoder_writeInt, { ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
	{ "json_encoder_writeDouble", (void *) json_encoder_writeDouble, { ArgType::ptr_t, ArgType::double_t }, ArgType::void_t },
	{ "json_encoder_writeString", (void *) json_encoder_writeString, { ArgType::ptr_t, ArgType::ptr_t, ArgType::int32_t }, ArgType::void_t },
};

int populatePlaydateApiStruct(void *api) {
	int offset = 0;
	auto PlaydateAPI = (PlaydateAPI_32 *) ((uintptr_t) api + offset);
	offset += sizeof(PlaydateAPI_32);
	auto playdate_sys = (playdate_sys_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->system = (decltype(PlaydateAPI->system)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sys_32);
	playdate_sys->realloc = decltype(playdate_sys->realloc)(FUNC_TABLE_ADDRESS + 0 * 2 + 1);
	playdate_sys->formatString = decltype(playdate_sys->formatString)(FUNC_TABLE_ADDRESS + 1 * 2 + 1);
	playdate_sys->logToConsole = decltype(playdate_sys->logToConsole)(FUNC_TABLE_ADDRESS + 2 * 2 + 1);
	playdate_sys->error = decltype(playdate_sys->error)(FUNC_TABLE_ADDRESS + 3 * 2 + 1);
	playdate_sys->getLanguage = decltype(playdate_sys->getLanguage)(FUNC_TABLE_ADDRESS + 4 * 2 + 1);
	playdate_sys->getCurrentTimeMilliseconds = decltype(playdate_sys->getCurrentTimeMilliseconds)(FUNC_TABLE_ADDRESS + 5 * 2 + 1);
	playdate_sys->getSecondsSinceEpoch = decltype(playdate_sys->getSecondsSinceEpoch)(FUNC_TABLE_ADDRESS + 6 * 2 + 1);
	playdate_sys->drawFPS = decltype(playdate_sys->drawFPS)(FUNC_TABLE_ADDRESS + 7 * 2 + 1);
	playdate_sys->setUpdateCallback = decltype(playdate_sys->setUpdateCallback)(FUNC_TABLE_ADDRESS + 8 * 2 + 1);
	playdate_sys->getButtonState = decltype(playdate_sys->getButtonState)(FUNC_TABLE_ADDRESS + 9 * 2 + 1);
	playdate_sys->setPeripheralsEnabled = decltype(playdate_sys->setPeripheralsEnabled)(FUNC_TABLE_ADDRESS + 10 * 2 + 1);
	playdate_sys->getAccelerometer = decltype(playdate_sys->getAccelerometer)(FUNC_TABLE_ADDRESS + 11 * 2 + 1);
	playdate_sys->getCrankChange = decltype(playdate_sys->getCrankChange)(FUNC_TABLE_ADDRESS + 12 * 2 + 1);
	playdate_sys->getCrankAngle = decltype(playdate_sys->getCrankAngle)(FUNC_TABLE_ADDRESS + 13 * 2 + 1);
	playdate_sys->isCrankDocked = decltype(playdate_sys->isCrankDocked)(FUNC_TABLE_ADDRESS + 14 * 2 + 1);
	playdate_sys->setCrankSoundsDisabled = decltype(playdate_sys->setCrankSoundsDisabled)(FUNC_TABLE_ADDRESS + 15 * 2 + 1);
	playdate_sys->getFlipped = decltype(playdate_sys->getFlipped)(FUNC_TABLE_ADDRESS + 16 * 2 + 1);
	playdate_sys->setAutoLockDisabled = decltype(playdate_sys->setAutoLockDisabled)(FUNC_TABLE_ADDRESS + 17 * 2 + 1);
	playdate_sys->setMenuImage = decltype(playdate_sys->setMenuImage)(FUNC_TABLE_ADDRESS + 18 * 2 + 1);
	playdate_sys->addMenuItem = decltype(playdate_sys->addMenuItem)(FUNC_TABLE_ADDRESS + 19 * 2 + 1);
	playdate_sys->addCheckmarkMenuItem = decltype(playdate_sys->addCheckmarkMenuItem)(FUNC_TABLE_ADDRESS + 20 * 2 + 1);
	playdate_sys->addOptionsMenuItem = decltype(playdate_sys->addOptionsMenuItem)(FUNC_TABLE_ADDRESS + 21 * 2 + 1);
	playdate_sys->removeAllMenuItems = decltype(playdate_sys->removeAllMenuItems)(FUNC_TABLE_ADDRESS + 22 * 2 + 1);
	playdate_sys->removeMenuItem = decltype(playdate_sys->removeMenuItem)(FUNC_TABLE_ADDRESS + 23 * 2 + 1);
	playdate_sys->getMenuItemValue = decltype(playdate_sys->getMenuItemValue)(FUNC_TABLE_ADDRESS + 24 * 2 + 1);
	playdate_sys->setMenuItemValue = decltype(playdate_sys->setMenuItemValue)(FUNC_TABLE_ADDRESS + 25 * 2 + 1);
	playdate_sys->getMenuItemTitle = decltype(playdate_sys->getMenuItemTitle)(FUNC_TABLE_ADDRESS + 26 * 2 + 1);
	playdate_sys->setMenuItemTitle = decltype(playdate_sys->setMenuItemTitle)(FUNC_TABLE_ADDRESS + 27 * 2 + 1);
	playdate_sys->getMenuItemUserdata = decltype(playdate_sys->getMenuItemUserdata)(FUNC_TABLE_ADDRESS + 28 * 2 + 1);
	playdate_sys->setMenuItemUserdata = decltype(playdate_sys->setMenuItemUserdata)(FUNC_TABLE_ADDRESS + 29 * 2 + 1);
	playdate_sys->getReduceFlashing = decltype(playdate_sys->getReduceFlashing)(FUNC_TABLE_ADDRESS + 30 * 2 + 1);
	playdate_sys->getElapsedTime = decltype(playdate_sys->getElapsedTime)(FUNC_TABLE_ADDRESS + 31 * 2 + 1);
	playdate_sys->resetElapsedTime = decltype(playdate_sys->resetElapsedTime)(FUNC_TABLE_ADDRESS + 32 * 2 + 1);
	playdate_sys->getBatteryPercentage = decltype(playdate_sys->getBatteryPercentage)(FUNC_TABLE_ADDRESS + 33 * 2 + 1);
	playdate_sys->getBatteryVoltage = decltype(playdate_sys->getBatteryVoltage)(FUNC_TABLE_ADDRESS + 34 * 2 + 1);
	playdate_sys->getTimezoneOffset = decltype(playdate_sys->getTimezoneOffset)(FUNC_TABLE_ADDRESS + 35 * 2 + 1);
	playdate_sys->shouldDisplay24HourTime = decltype(playdate_sys->shouldDisplay24HourTime)(FUNC_TABLE_ADDRESS + 36 * 2 + 1);
	playdate_sys->convertEpochToDateTime = decltype(playdate_sys->convertEpochToDateTime)(FUNC_TABLE_ADDRESS + 37 * 2 + 1);
	playdate_sys->convertDateTimeToEpoch = decltype(playdate_sys->convertDateTimeToEpoch)(FUNC_TABLE_ADDRESS + 38 * 2 + 1);
	playdate_sys->clearICache = decltype(playdate_sys->clearICache)(FUNC_TABLE_ADDRESS + 39 * 2 + 1);
	auto playdate_file = (playdate_file_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->file = (decltype(PlaydateAPI->file)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_file_32);
	playdate_file->geterr = decltype(playdate_file->geterr)(FUNC_TABLE_ADDRESS + 40 * 2 + 1);
	playdate_file->listfiles = decltype(playdate_file->listfiles)(FUNC_TABLE_ADDRESS + 41 * 2 + 1);
	playdate_file->stat = decltype(playdate_file->stat)(FUNC_TABLE_ADDRESS + 42 * 2 + 1);
	playdate_file->mkdir = decltype(playdate_file->mkdir)(FUNC_TABLE_ADDRESS + 43 * 2 + 1);
	playdate_file->unlink = decltype(playdate_file->unlink)(FUNC_TABLE_ADDRESS + 44 * 2 + 1);
	playdate_file->rename = decltype(playdate_file->rename)(FUNC_TABLE_ADDRESS + 45 * 2 + 1);
	playdate_file->open = decltype(playdate_file->open)(FUNC_TABLE_ADDRESS + 46 * 2 + 1);
	playdate_file->close = decltype(playdate_file->close)(FUNC_TABLE_ADDRESS + 47 * 2 + 1);
	playdate_file->read = decltype(playdate_file->read)(FUNC_TABLE_ADDRESS + 48 * 2 + 1);
	playdate_file->write = decltype(playdate_file->write)(FUNC_TABLE_ADDRESS + 49 * 2 + 1);
	playdate_file->flush = decltype(playdate_file->flush)(FUNC_TABLE_ADDRESS + 50 * 2 + 1);
	playdate_file->tell = decltype(playdate_file->tell)(FUNC_TABLE_ADDRESS + 51 * 2 + 1);
	playdate_file->seek = decltype(playdate_file->seek)(FUNC_TABLE_ADDRESS + 52 * 2 + 1);
	auto playdate_graphics = (playdate_graphics_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->graphics = (decltype(PlaydateAPI->graphics)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_graphics_32);
	playdate_graphics->clear = decltype(playdate_graphics->clear)(FUNC_TABLE_ADDRESS + 61 * 2 + 1);
	playdate_graphics->setBackgroundColor = decltype(playdate_graphics->setBackgroundColor)(FUNC_TABLE_ADDRESS + 62 * 2 + 1);
	playdate_graphics->setStencil = decltype(playdate_graphics->setStencil)(FUNC_TABLE_ADDRESS + 63 * 2 + 1);
	playdate_graphics->setDrawMode = decltype(playdate_graphics->setDrawMode)(FUNC_TABLE_ADDRESS + 64 * 2 + 1);
	playdate_graphics->setDrawOffset = decltype(playdate_graphics->setDrawOffset)(FUNC_TABLE_ADDRESS + 65 * 2 + 1);
	playdate_graphics->setClipRect = decltype(playdate_graphics->setClipRect)(FUNC_TABLE_ADDRESS + 66 * 2 + 1);
	playdate_graphics->clearClipRect = decltype(playdate_graphics->clearClipRect)(FUNC_TABLE_ADDRESS + 67 * 2 + 1);
	playdate_graphics->setLineCapStyle = decltype(playdate_graphics->setLineCapStyle)(FUNC_TABLE_ADDRESS + 68 * 2 + 1);
	playdate_graphics->setFont = decltype(playdate_graphics->setFont)(FUNC_TABLE_ADDRESS + 69 * 2 + 1);
	playdate_graphics->setTextTracking = decltype(playdate_graphics->setTextTracking)(FUNC_TABLE_ADDRESS + 70 * 2 + 1);
	playdate_graphics->pushContext = decltype(playdate_graphics->pushContext)(FUNC_TABLE_ADDRESS + 71 * 2 + 1);
	playdate_graphics->popContext = decltype(playdate_graphics->popContext)(FUNC_TABLE_ADDRESS + 72 * 2 + 1);
	playdate_graphics->drawBitmap = decltype(playdate_graphics->drawBitmap)(FUNC_TABLE_ADDRESS + 73 * 2 + 1);
	playdate_graphics->tileBitmap = decltype(playdate_graphics->tileBitmap)(FUNC_TABLE_ADDRESS + 74 * 2 + 1);
	playdate_graphics->drawLine = decltype(playdate_graphics->drawLine)(FUNC_TABLE_ADDRESS + 75 * 2 + 1);
	playdate_graphics->fillTriangle = decltype(playdate_graphics->fillTriangle)(FUNC_TABLE_ADDRESS + 76 * 2 + 1);
	playdate_graphics->drawRect = decltype(playdate_graphics->drawRect)(FUNC_TABLE_ADDRESS + 77 * 2 + 1);
	playdate_graphics->fillRect = decltype(playdate_graphics->fillRect)(FUNC_TABLE_ADDRESS + 78 * 2 + 1);
	playdate_graphics->drawEllipse = decltype(playdate_graphics->drawEllipse)(FUNC_TABLE_ADDRESS + 79 * 2 + 1);
	playdate_graphics->fillEllipse = decltype(playdate_graphics->fillEllipse)(FUNC_TABLE_ADDRESS + 80 * 2 + 1);
	playdate_graphics->drawScaledBitmap = decltype(playdate_graphics->drawScaledBitmap)(FUNC_TABLE_ADDRESS + 81 * 2 + 1);
	playdate_graphics->drawText = decltype(playdate_graphics->drawText)(FUNC_TABLE_ADDRESS + 82 * 2 + 1);
	playdate_graphics->newBitmap = decltype(playdate_graphics->newBitmap)(FUNC_TABLE_ADDRESS + 83 * 2 + 1);
	playdate_graphics->freeBitmap = decltype(playdate_graphics->freeBitmap)(FUNC_TABLE_ADDRESS + 84 * 2 + 1);
	playdate_graphics->loadBitmap = decltype(playdate_graphics->loadBitmap)(FUNC_TABLE_ADDRESS + 85 * 2 + 1);
	playdate_graphics->copyBitmap = decltype(playdate_graphics->copyBitmap)(FUNC_TABLE_ADDRESS + 86 * 2 + 1);
	playdate_graphics->loadIntoBitmap = decltype(playdate_graphics->loadIntoBitmap)(FUNC_TABLE_ADDRESS + 87 * 2 + 1);
	playdate_graphics->getBitmapData = decltype(playdate_graphics->getBitmapData)(FUNC_TABLE_ADDRESS + 88 * 2 + 1);
	playdate_graphics->clearBitmap = decltype(playdate_graphics->clearBitmap)(FUNC_TABLE_ADDRESS + 89 * 2 + 1);
	playdate_graphics->rotatedBitmap = decltype(playdate_graphics->rotatedBitmap)(FUNC_TABLE_ADDRESS + 90 * 2 + 1);
	playdate_graphics->newBitmapTable = decltype(playdate_graphics->newBitmapTable)(FUNC_TABLE_ADDRESS + 91 * 2 + 1);
	playdate_graphics->freeBitmapTable = decltype(playdate_graphics->freeBitmapTable)(FUNC_TABLE_ADDRESS + 92 * 2 + 1);
	playdate_graphics->loadBitmapTable = decltype(playdate_graphics->loadBitmapTable)(FUNC_TABLE_ADDRESS + 93 * 2 + 1);
	playdate_graphics->loadIntoBitmapTable = decltype(playdate_graphics->loadIntoBitmapTable)(FUNC_TABLE_ADDRESS + 94 * 2 + 1);
	playdate_graphics->getTableBitmap = decltype(playdate_graphics->getTableBitmap)(FUNC_TABLE_ADDRESS + 95 * 2 + 1);
	playdate_graphics->loadFont = decltype(playdate_graphics->loadFont)(FUNC_TABLE_ADDRESS + 96 * 2 + 1);
	playdate_graphics->getFontPage = decltype(playdate_graphics->getFontPage)(FUNC_TABLE_ADDRESS + 97 * 2 + 1);
	playdate_graphics->getPageGlyph = decltype(playdate_graphics->getPageGlyph)(FUNC_TABLE_ADDRESS + 98 * 2 + 1);
	playdate_graphics->getGlyphKerning = decltype(playdate_graphics->getGlyphKerning)(FUNC_TABLE_ADDRESS + 99 * 2 + 1);
	playdate_graphics->getTextWidth = decltype(playdate_graphics->getTextWidth)(FUNC_TABLE_ADDRESS + 100 * 2 + 1);
	playdate_graphics->getFrame = decltype(playdate_graphics->getFrame)(FUNC_TABLE_ADDRESS + 101 * 2 + 1);
	playdate_graphics->getDisplayFrame = decltype(playdate_graphics->getDisplayFrame)(FUNC_TABLE_ADDRESS + 102 * 2 + 1);
	playdate_graphics->copyFrameBufferBitmap = decltype(playdate_graphics->copyFrameBufferBitmap)(FUNC_TABLE_ADDRESS + 103 * 2 + 1);
	playdate_graphics->markUpdatedRows = decltype(playdate_graphics->markUpdatedRows)(FUNC_TABLE_ADDRESS + 104 * 2 + 1);
	playdate_graphics->display = decltype(playdate_graphics->display)(FUNC_TABLE_ADDRESS + 105 * 2 + 1);
	playdate_graphics->setColorToPattern = decltype(playdate_graphics->setColorToPattern)(FUNC_TABLE_ADDRESS + 106 * 2 + 1);
	playdate_graphics->checkMaskCollision = decltype(playdate_graphics->checkMaskCollision)(FUNC_TABLE_ADDRESS + 107 * 2 + 1);
	playdate_graphics->setScreenClipRect = decltype(playdate_graphics->setScreenClipRect)(FUNC_TABLE_ADDRESS + 108 * 2 + 1);
	playdate_graphics->fillPolygon = decltype(playdate_graphics->fillPolygon)(FUNC_TABLE_ADDRESS + 109 * 2 + 1);
	playdate_graphics->getFontHeight = decltype(playdate_graphics->getFontHeight)(FUNC_TABLE_ADDRESS + 110 * 2 + 1);
	playdate_graphics->getDisplayBufferBitmap = decltype(playdate_graphics->getDisplayBufferBitmap)(FUNC_TABLE_ADDRESS + 111 * 2 + 1);
	playdate_graphics->drawRotatedBitmap = decltype(playdate_graphics->drawRotatedBitmap)(FUNC_TABLE_ADDRESS + 112 * 2 + 1);
	playdate_graphics->setTextLeading = decltype(playdate_graphics->setTextLeading)(FUNC_TABLE_ADDRESS + 113 * 2 + 1);
	playdate_graphics->setBitmapMask = decltype(playdate_graphics->setBitmapMask)(FUNC_TABLE_ADDRESS + 114 * 2 + 1);
	playdate_graphics->getBitmapMask = decltype(playdate_graphics->getBitmapMask)(FUNC_TABLE_ADDRESS + 115 * 2 + 1);
	playdate_graphics->setStencilImage = decltype(playdate_graphics->setStencilImage)(FUNC_TABLE_ADDRESS + 116 * 2 + 1);
	playdate_graphics->makeFontFromData = decltype(playdate_graphics->makeFontFromData)(FUNC_TABLE_ADDRESS + 117 * 2 + 1);
	auto playdate_video = (playdate_video_32 *) ((uintptr_t) api + offset);
	playdate_graphics->video = (decltype(playdate_graphics->video)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_video_32);
	playdate_video->loadVideo = decltype(playdate_video->loadVideo)(FUNC_TABLE_ADDRESS + 53 * 2 + 1);
	playdate_video->freePlayer = decltype(playdate_video->freePlayer)(FUNC_TABLE_ADDRESS + 54 * 2 + 1);
	playdate_video->setContext = decltype(playdate_video->setContext)(FUNC_TABLE_ADDRESS + 55 * 2 + 1);
	playdate_video->useScreenContext = decltype(playdate_video->useScreenContext)(FUNC_TABLE_ADDRESS + 56 * 2 + 1);
	playdate_video->renderFrame = decltype(playdate_video->renderFrame)(FUNC_TABLE_ADDRESS + 57 * 2 + 1);
	playdate_video->getError = decltype(playdate_video->getError)(FUNC_TABLE_ADDRESS + 58 * 2 + 1);
	playdate_video->getInfo = decltype(playdate_video->getInfo)(FUNC_TABLE_ADDRESS + 59 * 2 + 1);
	playdate_video->getContext = decltype(playdate_video->getContext)(FUNC_TABLE_ADDRESS + 60 * 2 + 1);
	auto playdate_sprite = (playdate_sprite_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->sprite = (decltype(PlaydateAPI->sprite)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sprite_32);
	playdate_sprite->setAlwaysRedraw = decltype(playdate_sprite->setAlwaysRedraw)(FUNC_TABLE_ADDRESS + 118 * 2 + 1);
	playdate_sprite->addDirtyRect = decltype(playdate_sprite->addDirtyRect)(FUNC_TABLE_ADDRESS + 119 * 2 + 1);
	playdate_sprite->drawSprites = decltype(playdate_sprite->drawSprites)(FUNC_TABLE_ADDRESS + 120 * 2 + 1);
	playdate_sprite->updateAndDrawSprites = decltype(playdate_sprite->updateAndDrawSprites)(FUNC_TABLE_ADDRESS + 121 * 2 + 1);
	playdate_sprite->newSprite = decltype(playdate_sprite->newSprite)(FUNC_TABLE_ADDRESS + 122 * 2 + 1);
	playdate_sprite->freeSprite = decltype(playdate_sprite->freeSprite)(FUNC_TABLE_ADDRESS + 123 * 2 + 1);
	playdate_sprite->copy = decltype(playdate_sprite->copy)(FUNC_TABLE_ADDRESS + 124 * 2 + 1);
	playdate_sprite->addSprite = decltype(playdate_sprite->addSprite)(FUNC_TABLE_ADDRESS + 125 * 2 + 1);
	playdate_sprite->removeSprite = decltype(playdate_sprite->removeSprite)(FUNC_TABLE_ADDRESS + 126 * 2 + 1);
	playdate_sprite->removeSprites = decltype(playdate_sprite->removeSprites)(FUNC_TABLE_ADDRESS + 127 * 2 + 1);
	playdate_sprite->removeAllSprites = decltype(playdate_sprite->removeAllSprites)(FUNC_TABLE_ADDRESS + 128 * 2 + 1);
	playdate_sprite->getSpriteCount = decltype(playdate_sprite->getSpriteCount)(FUNC_TABLE_ADDRESS + 129 * 2 + 1);
	playdate_sprite->setBounds = decltype(playdate_sprite->setBounds)(FUNC_TABLE_ADDRESS + 130 * 2 + 1);
	playdate_sprite->getBounds = decltype(playdate_sprite->getBounds)(FUNC_TABLE_ADDRESS + 131 * 2 + 1);
	playdate_sprite->moveTo = decltype(playdate_sprite->moveTo)(FUNC_TABLE_ADDRESS + 132 * 2 + 1);
	playdate_sprite->moveBy = decltype(playdate_sprite->moveBy)(FUNC_TABLE_ADDRESS + 133 * 2 + 1);
	playdate_sprite->setImage = decltype(playdate_sprite->setImage)(FUNC_TABLE_ADDRESS + 134 * 2 + 1);
	playdate_sprite->getImage = decltype(playdate_sprite->getImage)(FUNC_TABLE_ADDRESS + 135 * 2 + 1);
	playdate_sprite->setSize = decltype(playdate_sprite->setSize)(FUNC_TABLE_ADDRESS + 136 * 2 + 1);
	playdate_sprite->setZIndex = decltype(playdate_sprite->setZIndex)(FUNC_TABLE_ADDRESS + 137 * 2 + 1);
	playdate_sprite->getZIndex = decltype(playdate_sprite->getZIndex)(FUNC_TABLE_ADDRESS + 138 * 2 + 1);
	playdate_sprite->setDrawMode = decltype(playdate_sprite->setDrawMode)(FUNC_TABLE_ADDRESS + 139 * 2 + 1);
	playdate_sprite->setImageFlip = decltype(playdate_sprite->setImageFlip)(FUNC_TABLE_ADDRESS + 140 * 2 + 1);
	playdate_sprite->getImageFlip = decltype(playdate_sprite->getImageFlip)(FUNC_TABLE_ADDRESS + 141 * 2 + 1);
	playdate_sprite->setStencil = decltype(playdate_sprite->setStencil)(FUNC_TABLE_ADDRESS + 142 * 2 + 1);
	playdate_sprite->setClipRect = decltype(playdate_sprite->setClipRect)(FUNC_TABLE_ADDRESS + 143 * 2 + 1);
	playdate_sprite->clearClipRect = decltype(playdate_sprite->clearClipRect)(FUNC_TABLE_ADDRESS + 144 * 2 + 1);
	playdate_sprite->setClipRectsInRange = decltype(playdate_sprite->setClipRectsInRange)(FUNC_TABLE_ADDRESS + 145 * 2 + 1);
	playdate_sprite->clearClipRectsInRange = decltype(playdate_sprite->clearClipRectsInRange)(FUNC_TABLE_ADDRESS + 146 * 2 + 1);
	playdate_sprite->setUpdatesEnabled = decltype(playdate_sprite->setUpdatesEnabled)(FUNC_TABLE_ADDRESS + 147 * 2 + 1);
	playdate_sprite->updatesEnabled = decltype(playdate_sprite->updatesEnabled)(FUNC_TABLE_ADDRESS + 148 * 2 + 1);
	playdate_sprite->setCollisionsEnabled = decltype(playdate_sprite->setCollisionsEnabled)(FUNC_TABLE_ADDRESS + 149 * 2 + 1);
	playdate_sprite->collisionsEnabled = decltype(playdate_sprite->collisionsEnabled)(FUNC_TABLE_ADDRESS + 150 * 2 + 1);
	playdate_sprite->setVisible = decltype(playdate_sprite->setVisible)(FUNC_TABLE_ADDRESS + 151 * 2 + 1);
	playdate_sprite->isVisible = decltype(playdate_sprite->isVisible)(FUNC_TABLE_ADDRESS + 152 * 2 + 1);
	playdate_sprite->setOpaque = decltype(playdate_sprite->setOpaque)(FUNC_TABLE_ADDRESS + 153 * 2 + 1);
	playdate_sprite->markDirty = decltype(playdate_sprite->markDirty)(FUNC_TABLE_ADDRESS + 154 * 2 + 1);
	playdate_sprite->setTag = decltype(playdate_sprite->setTag)(FUNC_TABLE_ADDRESS + 155 * 2 + 1);
	playdate_sprite->getTag = decltype(playdate_sprite->getTag)(FUNC_TABLE_ADDRESS + 156 * 2 + 1);
	playdate_sprite->setIgnoresDrawOffset = decltype(playdate_sprite->setIgnoresDrawOffset)(FUNC_TABLE_ADDRESS + 157 * 2 + 1);
	playdate_sprite->setUpdateFunction = decltype(playdate_sprite->setUpdateFunction)(FUNC_TABLE_ADDRESS + 158 * 2 + 1);
	playdate_sprite->setDrawFunction = decltype(playdate_sprite->setDrawFunction)(FUNC_TABLE_ADDRESS + 159 * 2 + 1);
	playdate_sprite->getPosition = decltype(playdate_sprite->getPosition)(FUNC_TABLE_ADDRESS + 160 * 2 + 1);
	playdate_sprite->resetCollisionWorld = decltype(playdate_sprite->resetCollisionWorld)(FUNC_TABLE_ADDRESS + 161 * 2 + 1);
	playdate_sprite->setCollideRect = decltype(playdate_sprite->setCollideRect)(FUNC_TABLE_ADDRESS + 162 * 2 + 1);
	playdate_sprite->getCollideRect = decltype(playdate_sprite->getCollideRect)(FUNC_TABLE_ADDRESS + 163 * 2 + 1);
	playdate_sprite->clearCollideRect = decltype(playdate_sprite->clearCollideRect)(FUNC_TABLE_ADDRESS + 164 * 2 + 1);
	playdate_sprite->setCollisionResponseFunction = decltype(playdate_sprite->setCollisionResponseFunction)(FUNC_TABLE_ADDRESS + 165 * 2 + 1);
	playdate_sprite->checkCollisions = decltype(playdate_sprite->checkCollisions)(FUNC_TABLE_ADDRESS + 166 * 2 + 1);
	playdate_sprite->moveWithCollisions = decltype(playdate_sprite->moveWithCollisions)(FUNC_TABLE_ADDRESS + 167 * 2 + 1);
	playdate_sprite->querySpritesAtPoint = decltype(playdate_sprite->querySpritesAtPoint)(FUNC_TABLE_ADDRESS + 168 * 2 + 1);
	playdate_sprite->querySpritesInRect = decltype(playdate_sprite->querySpritesInRect)(FUNC_TABLE_ADDRESS + 169 * 2 + 1);
	playdate_sprite->querySpritesAlongLine = decltype(playdate_sprite->querySpritesAlongLine)(FUNC_TABLE_ADDRESS + 170 * 2 + 1);
	playdate_sprite->querySpriteInfoAlongLine = decltype(playdate_sprite->querySpriteInfoAlongLine)(FUNC_TABLE_ADDRESS + 171 * 2 + 1);
	playdate_sprite->overlappingSprites = decltype(playdate_sprite->overlappingSprites)(FUNC_TABLE_ADDRESS + 172 * 2 + 1);
	playdate_sprite->allOverlappingSprites = decltype(playdate_sprite->allOverlappingSprites)(FUNC_TABLE_ADDRESS + 173 * 2 + 1);
	playdate_sprite->setStencilPattern = decltype(playdate_sprite->setStencilPattern)(FUNC_TABLE_ADDRESS + 174 * 2 + 1);
	playdate_sprite->clearStencil = decltype(playdate_sprite->clearStencil)(FUNC_TABLE_ADDRESS + 175 * 2 + 1);
	playdate_sprite->setUserdata = decltype(playdate_sprite->setUserdata)(FUNC_TABLE_ADDRESS + 176 * 2 + 1);
	playdate_sprite->getUserdata = decltype(playdate_sprite->getUserdata)(FUNC_TABLE_ADDRESS + 177 * 2 + 1);
	playdate_sprite->setStencilImage = decltype(playdate_sprite->setStencilImage)(FUNC_TABLE_ADDRESS + 178 * 2 + 1);
	auto playdate_display = (playdate_display_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->display = (decltype(PlaydateAPI->display)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_display_32);
	playdate_display->getWidth = decltype(playdate_display->getWidth)(FUNC_TABLE_ADDRESS + 179 * 2 + 1);
	playdate_display->getHeight = decltype(playdate_display->getHeight)(FUNC_TABLE_ADDRESS + 180 * 2 + 1);
	playdate_display->setRefreshRate = decltype(playdate_display->setRefreshRate)(FUNC_TABLE_ADDRESS + 181 * 2 + 1);
	playdate_display->setInverted = decltype(playdate_display->setInverted)(FUNC_TABLE_ADDRESS + 182 * 2 + 1);
	playdate_display->setScale = decltype(playdate_display->setScale)(FUNC_TABLE_ADDRESS + 183 * 2 + 1);
	playdate_display->setMosaic = decltype(playdate_display->setMosaic)(FUNC_TABLE_ADDRESS + 184 * 2 + 1);
	playdate_display->setFlipped = decltype(playdate_display->setFlipped)(FUNC_TABLE_ADDRESS + 185 * 2 + 1);
	playdate_display->setOffset = decltype(playdate_display->setOffset)(FUNC_TABLE_ADDRESS + 186 * 2 + 1);
	auto playdate_sound = (playdate_sound_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->sound = (decltype(PlaydateAPI->sound)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_32);
	playdate_sound->getCurrentTime = decltype(playdate_sound->getCurrentTime)(FUNC_TABLE_ADDRESS + 413 * 2 + 1);
	playdate_sound->addSource = decltype(playdate_sound->addSource)(FUNC_TABLE_ADDRESS + 414 * 2 + 1);
	playdate_sound->getDefaultChannel = decltype(playdate_sound->getDefaultChannel)(FUNC_TABLE_ADDRESS + 415 * 2 + 1);
	playdate_sound->addChannel = decltype(playdate_sound->addChannel)(FUNC_TABLE_ADDRESS + 416 * 2 + 1);
	playdate_sound->removeChannel = decltype(playdate_sound->removeChannel)(FUNC_TABLE_ADDRESS + 417 * 2 + 1);
	playdate_sound->setMicCallback = decltype(playdate_sound->setMicCallback)(FUNC_TABLE_ADDRESS + 418 * 2 + 1);
	playdate_sound->getHeadphoneState = decltype(playdate_sound->getHeadphoneState)(FUNC_TABLE_ADDRESS + 419 * 2 + 1);
	playdate_sound->setOutputsActive = decltype(playdate_sound->setOutputsActive)(FUNC_TABLE_ADDRESS + 420 * 2 + 1);
	playdate_sound->removeSource = decltype(playdate_sound->removeSource)(FUNC_TABLE_ADDRESS + 421 * 2 + 1);
	auto playdate_sound_channel = (playdate_sound_channel_32 *) ((uintptr_t) api + offset);
	playdate_sound->channel = (decltype(playdate_sound->channel)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_channel_32);
	playdate_sound_channel->newChannel = decltype(playdate_sound_channel->newChannel)(FUNC_TABLE_ADDRESS + 187 * 2 + 1);
	playdate_sound_channel->freeChannel = decltype(playdate_sound_channel->freeChannel)(FUNC_TABLE_ADDRESS + 188 * 2 + 1);
	playdate_sound_channel->addSource = decltype(playdate_sound_channel->addSource)(FUNC_TABLE_ADDRESS + 189 * 2 + 1);
	playdate_sound_channel->removeSource = decltype(playdate_sound_channel->removeSource)(FUNC_TABLE_ADDRESS + 190 * 2 + 1);
	playdate_sound_channel->addCallbackSource = decltype(playdate_sound_channel->addCallbackSource)(FUNC_TABLE_ADDRESS + 191 * 2 + 1);
	playdate_sound_channel->addEffect = decltype(playdate_sound_channel->addEffect)(FUNC_TABLE_ADDRESS + 192 * 2 + 1);
	playdate_sound_channel->removeEffect = decltype(playdate_sound_channel->removeEffect)(FUNC_TABLE_ADDRESS + 193 * 2 + 1);
	playdate_sound_channel->setVolume = decltype(playdate_sound_channel->setVolume)(FUNC_TABLE_ADDRESS + 194 * 2 + 1);
	playdate_sound_channel->getVolume = decltype(playdate_sound_channel->getVolume)(FUNC_TABLE_ADDRESS + 195 * 2 + 1);
	playdate_sound_channel->setVolumeModulator = decltype(playdate_sound_channel->setVolumeModulator)(FUNC_TABLE_ADDRESS + 196 * 2 + 1);
	playdate_sound_channel->getVolumeModulator = decltype(playdate_sound_channel->getVolumeModulator)(FUNC_TABLE_ADDRESS + 197 * 2 + 1);
	playdate_sound_channel->setPan = decltype(playdate_sound_channel->setPan)(FUNC_TABLE_ADDRESS + 198 * 2 + 1);
	playdate_sound_channel->setPanModulator = decltype(playdate_sound_channel->setPanModulator)(FUNC_TABLE_ADDRESS + 199 * 2 + 1);
	playdate_sound_channel->getPanModulator = decltype(playdate_sound_channel->getPanModulator)(FUNC_TABLE_ADDRESS + 200 * 2 + 1);
	playdate_sound_channel->getDryLevelSignal = decltype(playdate_sound_channel->getDryLevelSignal)(FUNC_TABLE_ADDRESS + 201 * 2 + 1);
	playdate_sound_channel->getWetLevelSignal = decltype(playdate_sound_channel->getWetLevelSignal)(FUNC_TABLE_ADDRESS + 202 * 2 + 1);
	auto playdate_sound_fileplayer = (playdate_sound_fileplayer_32 *) ((uintptr_t) api + offset);
	playdate_sound->fileplayer = (decltype(playdate_sound->fileplayer)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_fileplayer_32);
	playdate_sound_fileplayer->newPlayer = decltype(playdate_sound_fileplayer->newPlayer)(FUNC_TABLE_ADDRESS + 203 * 2 + 1);
	playdate_sound_fileplayer->freePlayer = decltype(playdate_sound_fileplayer->freePlayer)(FUNC_TABLE_ADDRESS + 204 * 2 + 1);
	playdate_sound_fileplayer->loadIntoPlayer = decltype(playdate_sound_fileplayer->loadIntoPlayer)(FUNC_TABLE_ADDRESS + 205 * 2 + 1);
	playdate_sound_fileplayer->setBufferLength = decltype(playdate_sound_fileplayer->setBufferLength)(FUNC_TABLE_ADDRESS + 206 * 2 + 1);
	playdate_sound_fileplayer->play = decltype(playdate_sound_fileplayer->play)(FUNC_TABLE_ADDRESS + 207 * 2 + 1);
	playdate_sound_fileplayer->isPlaying = decltype(playdate_sound_fileplayer->isPlaying)(FUNC_TABLE_ADDRESS + 208 * 2 + 1);
	playdate_sound_fileplayer->pause = decltype(playdate_sound_fileplayer->pause)(FUNC_TABLE_ADDRESS + 209 * 2 + 1);
	playdate_sound_fileplayer->stop = decltype(playdate_sound_fileplayer->stop)(FUNC_TABLE_ADDRESS + 210 * 2 + 1);
	playdate_sound_fileplayer->setVolume = decltype(playdate_sound_fileplayer->setVolume)(FUNC_TABLE_ADDRESS + 211 * 2 + 1);
	playdate_sound_fileplayer->getVolume = decltype(playdate_sound_fileplayer->getVolume)(FUNC_TABLE_ADDRESS + 212 * 2 + 1);
	playdate_sound_fileplayer->getLength = decltype(playdate_sound_fileplayer->getLength)(FUNC_TABLE_ADDRESS + 213 * 2 + 1);
	playdate_sound_fileplayer->setOffset = decltype(playdate_sound_fileplayer->setOffset)(FUNC_TABLE_ADDRESS + 214 * 2 + 1);
	playdate_sound_fileplayer->setRate = decltype(playdate_sound_fileplayer->setRate)(FUNC_TABLE_ADDRESS + 215 * 2 + 1);
	playdate_sound_fileplayer->setLoopRange = decltype(playdate_sound_fileplayer->setLoopRange)(FUNC_TABLE_ADDRESS + 216 * 2 + 1);
	playdate_sound_fileplayer->didUnderrun = decltype(playdate_sound_fileplayer->didUnderrun)(FUNC_TABLE_ADDRESS + 217 * 2 + 1);
	playdate_sound_fileplayer->setFinishCallback = decltype(playdate_sound_fileplayer->setFinishCallback)(FUNC_TABLE_ADDRESS + 218 * 2 + 1);
	playdate_sound_fileplayer->setLoopCallback = decltype(playdate_sound_fileplayer->setLoopCallback)(FUNC_TABLE_ADDRESS + 219 * 2 + 1);
	playdate_sound_fileplayer->getOffset = decltype(playdate_sound_fileplayer->getOffset)(FUNC_TABLE_ADDRESS + 220 * 2 + 1);
	playdate_sound_fileplayer->getRate = decltype(playdate_sound_fileplayer->getRate)(FUNC_TABLE_ADDRESS + 221 * 2 + 1);
	playdate_sound_fileplayer->setStopOnUnderrun = decltype(playdate_sound_fileplayer->setStopOnUnderrun)(FUNC_TABLE_ADDRESS + 222 * 2 + 1);
	playdate_sound_fileplayer->fadeVolume = decltype(playdate_sound_fileplayer->fadeVolume)(FUNC_TABLE_ADDRESS + 223 * 2 + 1);
	playdate_sound_fileplayer->setMP3StreamSource = decltype(playdate_sound_fileplayer->setMP3StreamSource)(FUNC_TABLE_ADDRESS + 224 * 2 + 1);
	auto playdate_sound_sample = (playdate_sound_sample_32 *) ((uintptr_t) api + offset);
	playdate_sound->sample = (decltype(playdate_sound->sample)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_sample_32);
	playdate_sound_sample->newSampleBuffer = decltype(playdate_sound_sample->newSampleBuffer)(FUNC_TABLE_ADDRESS + 225 * 2 + 1);
	playdate_sound_sample->loadIntoSample = decltype(playdate_sound_sample->loadIntoSample)(FUNC_TABLE_ADDRESS + 226 * 2 + 1);
	playdate_sound_sample->load = decltype(playdate_sound_sample->load)(FUNC_TABLE_ADDRESS + 227 * 2 + 1);
	playdate_sound_sample->newSampleFromData = decltype(playdate_sound_sample->newSampleFromData)(FUNC_TABLE_ADDRESS + 228 * 2 + 1);
	playdate_sound_sample->getData = decltype(playdate_sound_sample->getData)(FUNC_TABLE_ADDRESS + 229 * 2 + 1);
	playdate_sound_sample->freeSample = decltype(playdate_sound_sample->freeSample)(FUNC_TABLE_ADDRESS + 230 * 2 + 1);
	playdate_sound_sample->getLength = decltype(playdate_sound_sample->getLength)(FUNC_TABLE_ADDRESS + 231 * 2 + 1);
	auto playdate_sound_sampleplayer = (playdate_sound_sampleplayer_32 *) ((uintptr_t) api + offset);
	playdate_sound->sampleplayer = (decltype(playdate_sound->sampleplayer)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_sampleplayer_32);
	playdate_sound_sampleplayer->newPlayer = decltype(playdate_sound_sampleplayer->newPlayer)(FUNC_TABLE_ADDRESS + 232 * 2 + 1);
	playdate_sound_sampleplayer->freePlayer = decltype(playdate_sound_sampleplayer->freePlayer)(FUNC_TABLE_ADDRESS + 233 * 2 + 1);
	playdate_sound_sampleplayer->setSample = decltype(playdate_sound_sampleplayer->setSample)(FUNC_TABLE_ADDRESS + 234 * 2 + 1);
	playdate_sound_sampleplayer->play = decltype(playdate_sound_sampleplayer->play)(FUNC_TABLE_ADDRESS + 235 * 2 + 1);
	playdate_sound_sampleplayer->isPlaying = decltype(playdate_sound_sampleplayer->isPlaying)(FUNC_TABLE_ADDRESS + 236 * 2 + 1);
	playdate_sound_sampleplayer->stop = decltype(playdate_sound_sampleplayer->stop)(FUNC_TABLE_ADDRESS + 237 * 2 + 1);
	playdate_sound_sampleplayer->setVolume = decltype(playdate_sound_sampleplayer->setVolume)(FUNC_TABLE_ADDRESS + 238 * 2 + 1);
	playdate_sound_sampleplayer->getVolume = decltype(playdate_sound_sampleplayer->getVolume)(FUNC_TABLE_ADDRESS + 239 * 2 + 1);
	playdate_sound_sampleplayer->getLength = decltype(playdate_sound_sampleplayer->getLength)(FUNC_TABLE_ADDRESS + 240 * 2 + 1);
	playdate_sound_sampleplayer->setOffset = decltype(playdate_sound_sampleplayer->setOffset)(FUNC_TABLE_ADDRESS + 241 * 2 + 1);
	playdate_sound_sampleplayer->setRate = decltype(playdate_sound_sampleplayer->setRate)(FUNC_TABLE_ADDRESS + 242 * 2 + 1);
	playdate_sound_sampleplayer->setPlayRange = decltype(playdate_sound_sampleplayer->setPlayRange)(FUNC_TABLE_ADDRESS + 243 * 2 + 1);
	playdate_sound_sampleplayer->setFinishCallback = decltype(playdate_sound_sampleplayer->setFinishCallback)(FUNC_TABLE_ADDRESS + 244 * 2 + 1);
	playdate_sound_sampleplayer->setLoopCallback = decltype(playdate_sound_sampleplayer->setLoopCallback)(FUNC_TABLE_ADDRESS + 245 * 2 + 1);
	playdate_sound_sampleplayer->getOffset = decltype(playdate_sound_sampleplayer->getOffset)(FUNC_TABLE_ADDRESS + 246 * 2 + 1);
	playdate_sound_sampleplayer->getRate = decltype(playdate_sound_sampleplayer->getRate)(FUNC_TABLE_ADDRESS + 247 * 2 + 1);
	playdate_sound_sampleplayer->setPaused = decltype(playdate_sound_sampleplayer->setPaused)(FUNC_TABLE_ADDRESS + 248 * 2 + 1);
	auto playdate_sound_synth = (playdate_sound_synth_32 *) ((uintptr_t) api + offset);
	playdate_sound->synth = (decltype(playdate_sound->synth)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_synth_32);
	playdate_sound_synth->newSynth = decltype(playdate_sound_synth->newSynth)(FUNC_TABLE_ADDRESS + 249 * 2 + 1);
	playdate_sound_synth->freeSynth = decltype(playdate_sound_synth->freeSynth)(FUNC_TABLE_ADDRESS + 250 * 2 + 1);
	playdate_sound_synth->setWaveform = decltype(playdate_sound_synth->setWaveform)(FUNC_TABLE_ADDRESS + 251 * 2 + 1);
	playdate_sound_synth->setGenerator = decltype(playdate_sound_synth->setGenerator)(FUNC_TABLE_ADDRESS + 252 * 2 + 1);
	playdate_sound_synth->setSample = decltype(playdate_sound_synth->setSample)(FUNC_TABLE_ADDRESS + 253 * 2 + 1);
	playdate_sound_synth->setAttackTime = decltype(playdate_sound_synth->setAttackTime)(FUNC_TABLE_ADDRESS + 254 * 2 + 1);
	playdate_sound_synth->setDecayTime = decltype(playdate_sound_synth->setDecayTime)(FUNC_TABLE_ADDRESS + 255 * 2 + 1);
	playdate_sound_synth->setSustainLevel = decltype(playdate_sound_synth->setSustainLevel)(FUNC_TABLE_ADDRESS + 256 * 2 + 1);
	playdate_sound_synth->setReleaseTime = decltype(playdate_sound_synth->setReleaseTime)(FUNC_TABLE_ADDRESS + 257 * 2 + 1);
	playdate_sound_synth->setTranspose = decltype(playdate_sound_synth->setTranspose)(FUNC_TABLE_ADDRESS + 258 * 2 + 1);
	playdate_sound_synth->setFrequencyModulator = decltype(playdate_sound_synth->setFrequencyModulator)(FUNC_TABLE_ADDRESS + 259 * 2 + 1);
	playdate_sound_synth->getFrequencyModulator = decltype(playdate_sound_synth->getFrequencyModulator)(FUNC_TABLE_ADDRESS + 260 * 2 + 1);
	playdate_sound_synth->setAmplitudeModulator = decltype(playdate_sound_synth->setAmplitudeModulator)(FUNC_TABLE_ADDRESS + 261 * 2 + 1);
	playdate_sound_synth->getAmplitudeModulator = decltype(playdate_sound_synth->getAmplitudeModulator)(FUNC_TABLE_ADDRESS + 262 * 2 + 1);
	playdate_sound_synth->getParameterCount = decltype(playdate_sound_synth->getParameterCount)(FUNC_TABLE_ADDRESS + 263 * 2 + 1);
	playdate_sound_synth->setParameter = decltype(playdate_sound_synth->setParameter)(FUNC_TABLE_ADDRESS + 264 * 2 + 1);
	playdate_sound_synth->setParameterModulator = decltype(playdate_sound_synth->setParameterModulator)(FUNC_TABLE_ADDRESS + 265 * 2 + 1);
	playdate_sound_synth->getParameterModulator = decltype(playdate_sound_synth->getParameterModulator)(FUNC_TABLE_ADDRESS + 266 * 2 + 1);
	playdate_sound_synth->playNote = decltype(playdate_sound_synth->playNote)(FUNC_TABLE_ADDRESS + 267 * 2 + 1);
	playdate_sound_synth->playMIDINote = decltype(playdate_sound_synth->playMIDINote)(FUNC_TABLE_ADDRESS + 268 * 2 + 1);
	playdate_sound_synth->noteOff = decltype(playdate_sound_synth->noteOff)(FUNC_TABLE_ADDRESS + 269 * 2 + 1);
	playdate_sound_synth->stop = decltype(playdate_sound_synth->stop)(FUNC_TABLE_ADDRESS + 270 * 2 + 1);
	playdate_sound_synth->setVolume = decltype(playdate_sound_synth->setVolume)(FUNC_TABLE_ADDRESS + 271 * 2 + 1);
	playdate_sound_synth->getVolume = decltype(playdate_sound_synth->getVolume)(FUNC_TABLE_ADDRESS + 272 * 2 + 1);
	playdate_sound_synth->isPlaying = decltype(playdate_sound_synth->isPlaying)(FUNC_TABLE_ADDRESS + 273 * 2 + 1);
	playdate_sound_synth->getEnvelope = decltype(playdate_sound_synth->getEnvelope)(FUNC_TABLE_ADDRESS + 274 * 2 + 1);
	auto playdate_sound_sequence = (playdate_sound_sequence_32 *) ((uintptr_t) api + offset);
	playdate_sound->sequence = (decltype(playdate_sound->sequence)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_sequence_32);
	playdate_sound_sequence->newSequence = decltype(playdate_sound_sequence->newSequence)(FUNC_TABLE_ADDRESS + 275 * 2 + 1);
	playdate_sound_sequence->freeSequence = decltype(playdate_sound_sequence->freeSequence)(FUNC_TABLE_ADDRESS + 276 * 2 + 1);
	playdate_sound_sequence->loadMidiFile = decltype(playdate_sound_sequence->loadMidiFile)(FUNC_TABLE_ADDRESS + 277 * 2 + 1);
	playdate_sound_sequence->getTime = decltype(playdate_sound_sequence->getTime)(FUNC_TABLE_ADDRESS + 278 * 2 + 1);
	playdate_sound_sequence->setTime = decltype(playdate_sound_sequence->setTime)(FUNC_TABLE_ADDRESS + 279 * 2 + 1);
	playdate_sound_sequence->setLoops = decltype(playdate_sound_sequence->setLoops)(FUNC_TABLE_ADDRESS + 280 * 2 + 1);
	playdate_sound_sequence->getTempo = decltype(playdate_sound_sequence->getTempo)(FUNC_TABLE_ADDRESS + 281 * 2 + 1);
	playdate_sound_sequence->setTempo = decltype(playdate_sound_sequence->setTempo)(FUNC_TABLE_ADDRESS + 282 * 2 + 1);
	playdate_sound_sequence->getTrackCount = decltype(playdate_sound_sequence->getTrackCount)(FUNC_TABLE_ADDRESS + 283 * 2 + 1);
	playdate_sound_sequence->addTrack = decltype(playdate_sound_sequence->addTrack)(FUNC_TABLE_ADDRESS + 284 * 2 + 1);
	playdate_sound_sequence->getTrackAtIndex = decltype(playdate_sound_sequence->getTrackAtIndex)(FUNC_TABLE_ADDRESS + 285 * 2 + 1);
	playdate_sound_sequence->setTrackAtIndex = decltype(playdate_sound_sequence->setTrackAtIndex)(FUNC_TABLE_ADDRESS + 286 * 2 + 1);
	playdate_sound_sequence->allNotesOff = decltype(playdate_sound_sequence->allNotesOff)(FUNC_TABLE_ADDRESS + 287 * 2 + 1);
	playdate_sound_sequence->isPlaying = decltype(playdate_sound_sequence->isPlaying)(FUNC_TABLE_ADDRESS + 288 * 2 + 1);
	playdate_sound_sequence->getLength = decltype(playdate_sound_sequence->getLength)(FUNC_TABLE_ADDRESS + 289 * 2 + 1);
	playdate_sound_sequence->play = decltype(playdate_sound_sequence->play)(FUNC_TABLE_ADDRESS + 290 * 2 + 1);
	playdate_sound_sequence->stop = decltype(playdate_sound_sequence->stop)(FUNC_TABLE_ADDRESS + 291 * 2 + 1);
	playdate_sound_sequence->getCurrentStep = decltype(playdate_sound_sequence->getCurrentStep)(FUNC_TABLE_ADDRESS + 292 * 2 + 1);
	playdate_sound_sequence->setCurrentStep = decltype(playdate_sound_sequence->setCurrentStep)(FUNC_TABLE_ADDRESS + 293 * 2 + 1);
	auto playdate_sound_effect = (playdate_sound_effect_32 *) ((uintptr_t) api + offset);
	playdate_sound->effect = (decltype(playdate_sound->effect)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_32);
	playdate_sound_effect->newEffect = decltype(playdate_sound_effect->newEffect)(FUNC_TABLE_ADDRESS + 294 * 2 + 1);
	playdate_sound_effect->freeEffect = decltype(playdate_sound_effect->freeEffect)(FUNC_TABLE_ADDRESS + 295 * 2 + 1);
	playdate_sound_effect->setMix = decltype(playdate_sound_effect->setMix)(FUNC_TABLE_ADDRESS + 296 * 2 + 1);
	playdate_sound_effect->setMixModulator = decltype(playdate_sound_effect->setMixModulator)(FUNC_TABLE_ADDRESS + 297 * 2 + 1);
	playdate_sound_effect->getMixModulator = decltype(playdate_sound_effect->getMixModulator)(FUNC_TABLE_ADDRESS + 298 * 2 + 1);
	playdate_sound_effect->setUserdata = decltype(playdate_sound_effect->setUserdata)(FUNC_TABLE_ADDRESS + 299 * 2 + 1);
	playdate_sound_effect->getUserdata = decltype(playdate_sound_effect->getUserdata)(FUNC_TABLE_ADDRESS + 300 * 2 + 1);
	auto playdate_sound_effect_twopolefilter = (playdate_sound_effect_twopolefilter_32 *) ((uintptr_t) api + offset);
	playdate_sound_effect->twopolefilter = (decltype(playdate_sound_effect->twopolefilter)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_twopolefilter_32);
	playdate_sound_effect_twopolefilter->newFilter = decltype(playdate_sound_effect_twopolefilter->newFilter)(FUNC_TABLE_ADDRESS + 301 * 2 + 1);
	playdate_sound_effect_twopolefilter->freeFilter = decltype(playdate_sound_effect_twopolefilter->freeFilter)(FUNC_TABLE_ADDRESS + 302 * 2 + 1);
	playdate_sound_effect_twopolefilter->setType = decltype(playdate_sound_effect_twopolefilter->setType)(FUNC_TABLE_ADDRESS + 303 * 2 + 1);
	playdate_sound_effect_twopolefilter->setFrequency = decltype(playdate_sound_effect_twopolefilter->setFrequency)(FUNC_TABLE_ADDRESS + 304 * 2 + 1);
	playdate_sound_effect_twopolefilter->setFrequencyModulator = decltype(playdate_sound_effect_twopolefilter->setFrequencyModulator)(FUNC_TABLE_ADDRESS + 305 * 2 + 1);
	playdate_sound_effect_twopolefilter->getFrequencyModulator = decltype(playdate_sound_effect_twopolefilter->getFrequencyModulator)(FUNC_TABLE_ADDRESS + 306 * 2 + 1);
	playdate_sound_effect_twopolefilter->setGain = decltype(playdate_sound_effect_twopolefilter->setGain)(FUNC_TABLE_ADDRESS + 307 * 2 + 1);
	playdate_sound_effect_twopolefilter->setResonance = decltype(playdate_sound_effect_twopolefilter->setResonance)(FUNC_TABLE_ADDRESS + 308 * 2 + 1);
	playdate_sound_effect_twopolefilter->setResonanceModulator = decltype(playdate_sound_effect_twopolefilter->setResonanceModulator)(FUNC_TABLE_ADDRESS + 309 * 2 + 1);
	playdate_sound_effect_twopolefilter->getResonanceModulator = decltype(playdate_sound_effect_twopolefilter->getResonanceModulator)(FUNC_TABLE_ADDRESS + 310 * 2 + 1);
	auto playdate_sound_effect_onepolefilter = (playdate_sound_effect_onepolefilter_32 *) ((uintptr_t) api + offset);
	playdate_sound_effect->onepolefilter = (decltype(playdate_sound_effect->onepolefilter)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_onepolefilter_32);
	playdate_sound_effect_onepolefilter->newFilter = decltype(playdate_sound_effect_onepolefilter->newFilter)(FUNC_TABLE_ADDRESS + 311 * 2 + 1);
	playdate_sound_effect_onepolefilter->freeFilter = decltype(playdate_sound_effect_onepolefilter->freeFilter)(FUNC_TABLE_ADDRESS + 312 * 2 + 1);
	playdate_sound_effect_onepolefilter->setParameter = decltype(playdate_sound_effect_onepolefilter->setParameter)(FUNC_TABLE_ADDRESS + 313 * 2 + 1);
	playdate_sound_effect_onepolefilter->setParameterModulator = decltype(playdate_sound_effect_onepolefilter->setParameterModulator)(FUNC_TABLE_ADDRESS + 314 * 2 + 1);
	playdate_sound_effect_onepolefilter->getParameterModulator = decltype(playdate_sound_effect_onepolefilter->getParameterModulator)(FUNC_TABLE_ADDRESS + 315 * 2 + 1);
	auto playdate_sound_effect_bitcrusher = (playdate_sound_effect_bitcrusher_32 *) ((uintptr_t) api + offset);
	playdate_sound_effect->bitcrusher = (decltype(playdate_sound_effect->bitcrusher)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_bitcrusher_32);
	playdate_sound_effect_bitcrusher->newBitCrusher = decltype(playdate_sound_effect_bitcrusher->newBitCrusher)(FUNC_TABLE_ADDRESS + 316 * 2 + 1);
	playdate_sound_effect_bitcrusher->freeBitCrusher = decltype(playdate_sound_effect_bitcrusher->freeBitCrusher)(FUNC_TABLE_ADDRESS + 317 * 2 + 1);
	playdate_sound_effect_bitcrusher->setAmount = decltype(playdate_sound_effect_bitcrusher->setAmount)(FUNC_TABLE_ADDRESS + 318 * 2 + 1);
	playdate_sound_effect_bitcrusher->setAmountModulator = decltype(playdate_sound_effect_bitcrusher->setAmountModulator)(FUNC_TABLE_ADDRESS + 319 * 2 + 1);
	playdate_sound_effect_bitcrusher->getAmountModulator = decltype(playdate_sound_effect_bitcrusher->getAmountModulator)(FUNC_TABLE_ADDRESS + 320 * 2 + 1);
	playdate_sound_effect_bitcrusher->setUndersampling = decltype(playdate_sound_effect_bitcrusher->setUndersampling)(FUNC_TABLE_ADDRESS + 321 * 2 + 1);
	playdate_sound_effect_bitcrusher->setUndersampleModulator = decltype(playdate_sound_effect_bitcrusher->setUndersampleModulator)(FUNC_TABLE_ADDRESS + 322 * 2 + 1);
	playdate_sound_effect_bitcrusher->getUndersampleModulator = decltype(playdate_sound_effect_bitcrusher->getUndersampleModulator)(FUNC_TABLE_ADDRESS + 323 * 2 + 1);
	auto playdate_sound_effect_ringmodulator = (playdate_sound_effect_ringmodulator_32 *) ((uintptr_t) api + offset);
	playdate_sound_effect->ringmodulator = (decltype(playdate_sound_effect->ringmodulator)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_ringmodulator_32);
	playdate_sound_effect_ringmodulator->newRingmod = decltype(playdate_sound_effect_ringmodulator->newRingmod)(FUNC_TABLE_ADDRESS + 324 * 2 + 1);
	playdate_sound_effect_ringmodulator->freeRingmod = decltype(playdate_sound_effect_ringmodulator->freeRingmod)(FUNC_TABLE_ADDRESS + 325 * 2 + 1);
	playdate_sound_effect_ringmodulator->setFrequency = decltype(playdate_sound_effect_ringmodulator->setFrequency)(FUNC_TABLE_ADDRESS + 326 * 2 + 1);
	playdate_sound_effect_ringmodulator->setFrequencyModulator = decltype(playdate_sound_effect_ringmodulator->setFrequencyModulator)(FUNC_TABLE_ADDRESS + 327 * 2 + 1);
	playdate_sound_effect_ringmodulator->getFrequencyModulator = decltype(playdate_sound_effect_ringmodulator->getFrequencyModulator)(FUNC_TABLE_ADDRESS + 328 * 2 + 1);
	auto playdate_sound_effect_delayline = (playdate_sound_effect_delayline_32 *) ((uintptr_t) api + offset);
	playdate_sound_effect->delayline = (decltype(playdate_sound_effect->delayline)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_delayline_32);
	playdate_sound_effect_delayline->newDelayLine = decltype(playdate_sound_effect_delayline->newDelayLine)(FUNC_TABLE_ADDRESS + 329 * 2 + 1);
	playdate_sound_effect_delayline->freeDelayLine = decltype(playdate_sound_effect_delayline->freeDelayLine)(FUNC_TABLE_ADDRESS + 330 * 2 + 1);
	playdate_sound_effect_delayline->setLength = decltype(playdate_sound_effect_delayline->setLength)(FUNC_TABLE_ADDRESS + 331 * 2 + 1);
	playdate_sound_effect_delayline->setFeedback = decltype(playdate_sound_effect_delayline->setFeedback)(FUNC_TABLE_ADDRESS + 332 * 2 + 1);
	playdate_sound_effect_delayline->addTap = decltype(playdate_sound_effect_delayline->addTap)(FUNC_TABLE_ADDRESS + 333 * 2 + 1);
	playdate_sound_effect_delayline->freeTap = decltype(playdate_sound_effect_delayline->freeTap)(FUNC_TABLE_ADDRESS + 334 * 2 + 1);
	playdate_sound_effect_delayline->setTapDelay = decltype(playdate_sound_effect_delayline->setTapDelay)(FUNC_TABLE_ADDRESS + 335 * 2 + 1);
	playdate_sound_effect_delayline->setTapDelayModulator = decltype(playdate_sound_effect_delayline->setTapDelayModulator)(FUNC_TABLE_ADDRESS + 336 * 2 + 1);
	playdate_sound_effect_delayline->getTapDelayModulator = decltype(playdate_sound_effect_delayline->getTapDelayModulator)(FUNC_TABLE_ADDRESS + 337 * 2 + 1);
	playdate_sound_effect_delayline->setTapChannelsFlipped = decltype(playdate_sound_effect_delayline->setTapChannelsFlipped)(FUNC_TABLE_ADDRESS + 338 * 2 + 1);
	auto playdate_sound_effect_overdrive = (playdate_sound_effect_overdrive_32 *) ((uintptr_t) api + offset);
	playdate_sound_effect->overdrive = (decltype(playdate_sound_effect->overdrive)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_effect_overdrive_32);
	playdate_sound_effect_overdrive->newOverdrive = decltype(playdate_sound_effect_overdrive->newOverdrive)(FUNC_TABLE_ADDRESS + 339 * 2 + 1);
	playdate_sound_effect_overdrive->freeOverdrive = decltype(playdate_sound_effect_overdrive->freeOverdrive)(FUNC_TABLE_ADDRESS + 340 * 2 + 1);
	playdate_sound_effect_overdrive->setGain = decltype(playdate_sound_effect_overdrive->setGain)(FUNC_TABLE_ADDRESS + 341 * 2 + 1);
	playdate_sound_effect_overdrive->setLimit = decltype(playdate_sound_effect_overdrive->setLimit)(FUNC_TABLE_ADDRESS + 342 * 2 + 1);
	playdate_sound_effect_overdrive->setLimitModulator = decltype(playdate_sound_effect_overdrive->setLimitModulator)(FUNC_TABLE_ADDRESS + 343 * 2 + 1);
	playdate_sound_effect_overdrive->getLimitModulator = decltype(playdate_sound_effect_overdrive->getLimitModulator)(FUNC_TABLE_ADDRESS + 344 * 2 + 1);
	playdate_sound_effect_overdrive->setOffset = decltype(playdate_sound_effect_overdrive->setOffset)(FUNC_TABLE_ADDRESS + 345 * 2 + 1);
	playdate_sound_effect_overdrive->setOffsetModulator = decltype(playdate_sound_effect_overdrive->setOffsetModulator)(FUNC_TABLE_ADDRESS + 346 * 2 + 1);
	playdate_sound_effect_overdrive->getOffsetModulator = decltype(playdate_sound_effect_overdrive->getOffsetModulator)(FUNC_TABLE_ADDRESS + 347 * 2 + 1);
	auto playdate_sound_lfo = (playdate_sound_lfo_32 *) ((uintptr_t) api + offset);
	playdate_sound->lfo = (decltype(playdate_sound->lfo)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_lfo_32);
	playdate_sound_lfo->newLFO = decltype(playdate_sound_lfo->newLFO)(FUNC_TABLE_ADDRESS + 348 * 2 + 1);
	playdate_sound_lfo->freeLFO = decltype(playdate_sound_lfo->freeLFO)(FUNC_TABLE_ADDRESS + 349 * 2 + 1);
	playdate_sound_lfo->setType = decltype(playdate_sound_lfo->setType)(FUNC_TABLE_ADDRESS + 350 * 2 + 1);
	playdate_sound_lfo->setRate = decltype(playdate_sound_lfo->setRate)(FUNC_TABLE_ADDRESS + 351 * 2 + 1);
	playdate_sound_lfo->setPhase = decltype(playdate_sound_lfo->setPhase)(FUNC_TABLE_ADDRESS + 352 * 2 + 1);
	playdate_sound_lfo->setCenter = decltype(playdate_sound_lfo->setCenter)(FUNC_TABLE_ADDRESS + 353 * 2 + 1);
	playdate_sound_lfo->setDepth = decltype(playdate_sound_lfo->setDepth)(FUNC_TABLE_ADDRESS + 354 * 2 + 1);
	playdate_sound_lfo->setArpeggiation = decltype(playdate_sound_lfo->setArpeggiation)(FUNC_TABLE_ADDRESS + 355 * 2 + 1);
	playdate_sound_lfo->setFunction = decltype(playdate_sound_lfo->setFunction)(FUNC_TABLE_ADDRESS + 356 * 2 + 1);
	playdate_sound_lfo->setDelay = decltype(playdate_sound_lfo->setDelay)(FUNC_TABLE_ADDRESS + 357 * 2 + 1);
	playdate_sound_lfo->setRetrigger = decltype(playdate_sound_lfo->setRetrigger)(FUNC_TABLE_ADDRESS + 358 * 2 + 1);
	playdate_sound_lfo->getValue = decltype(playdate_sound_lfo->getValue)(FUNC_TABLE_ADDRESS + 359 * 2 + 1);
	playdate_sound_lfo->setGlobal = decltype(playdate_sound_lfo->setGlobal)(FUNC_TABLE_ADDRESS + 360 * 2 + 1);
	auto playdate_sound_envelope = (playdate_sound_envelope_32 *) ((uintptr_t) api + offset);
	playdate_sound->envelope = (decltype(playdate_sound->envelope)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_envelope_32);
	playdate_sound_envelope->newEnvelope = decltype(playdate_sound_envelope->newEnvelope)(FUNC_TABLE_ADDRESS + 361 * 2 + 1);
	playdate_sound_envelope->freeEnvelope = decltype(playdate_sound_envelope->freeEnvelope)(FUNC_TABLE_ADDRESS + 362 * 2 + 1);
	playdate_sound_envelope->setAttack = decltype(playdate_sound_envelope->setAttack)(FUNC_TABLE_ADDRESS + 363 * 2 + 1);
	playdate_sound_envelope->setDecay = decltype(playdate_sound_envelope->setDecay)(FUNC_TABLE_ADDRESS + 364 * 2 + 1);
	playdate_sound_envelope->setSustain = decltype(playdate_sound_envelope->setSustain)(FUNC_TABLE_ADDRESS + 365 * 2 + 1);
	playdate_sound_envelope->setRelease = decltype(playdate_sound_envelope->setRelease)(FUNC_TABLE_ADDRESS + 366 * 2 + 1);
	playdate_sound_envelope->setLegato = decltype(playdate_sound_envelope->setLegato)(FUNC_TABLE_ADDRESS + 367 * 2 + 1);
	playdate_sound_envelope->setRetrigger = decltype(playdate_sound_envelope->setRetrigger)(FUNC_TABLE_ADDRESS + 368 * 2 + 1);
	playdate_sound_envelope->getValue = decltype(playdate_sound_envelope->getValue)(FUNC_TABLE_ADDRESS + 369 * 2 + 1);
	playdate_sound_envelope->setCurvature = decltype(playdate_sound_envelope->setCurvature)(FUNC_TABLE_ADDRESS + 370 * 2 + 1);
	playdate_sound_envelope->setVelocitySensitivity = decltype(playdate_sound_envelope->setVelocitySensitivity)(FUNC_TABLE_ADDRESS + 371 * 2 + 1);
	playdate_sound_envelope->setRateScaling = decltype(playdate_sound_envelope->setRateScaling)(FUNC_TABLE_ADDRESS + 372 * 2 + 1);
	auto playdate_sound_source = (playdate_sound_source_32 *) ((uintptr_t) api + offset);
	playdate_sound->source = (decltype(playdate_sound->source)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_source_32);
	playdate_sound_source->setVolume = decltype(playdate_sound_source->setVolume)(FUNC_TABLE_ADDRESS + 373 * 2 + 1);
	playdate_sound_source->getVolume = decltype(playdate_sound_source->getVolume)(FUNC_TABLE_ADDRESS + 374 * 2 + 1);
	playdate_sound_source->isPlaying = decltype(playdate_sound_source->isPlaying)(FUNC_TABLE_ADDRESS + 375 * 2 + 1);
	playdate_sound_source->setFinishCallback = decltype(playdate_sound_source->setFinishCallback)(FUNC_TABLE_ADDRESS + 376 * 2 + 1);
	auto playdate_control_signal = (playdate_control_signal_32 *) ((uintptr_t) api + offset);
	playdate_sound->controlsignal = (decltype(playdate_sound->controlsignal)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_control_signal_32);
	playdate_control_signal->newSignal = decltype(playdate_control_signal->newSignal)(FUNC_TABLE_ADDRESS + 377 * 2 + 1);
	playdate_control_signal->freeSignal = decltype(playdate_control_signal->freeSignal)(FUNC_TABLE_ADDRESS + 378 * 2 + 1);
	playdate_control_signal->clearEvents = decltype(playdate_control_signal->clearEvents)(FUNC_TABLE_ADDRESS + 379 * 2 + 1);
	playdate_control_signal->addEvent = decltype(playdate_control_signal->addEvent)(FUNC_TABLE_ADDRESS + 380 * 2 + 1);
	playdate_control_signal->removeEvent = decltype(playdate_control_signal->removeEvent)(FUNC_TABLE_ADDRESS + 381 * 2 + 1);
	playdate_control_signal->getMIDIControllerNumber = decltype(playdate_control_signal->getMIDIControllerNumber)(FUNC_TABLE_ADDRESS + 382 * 2 + 1);
	auto playdate_sound_track = (playdate_sound_track_32 *) ((uintptr_t) api + offset);
	playdate_sound->track = (decltype(playdate_sound->track)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_track_32);
	playdate_sound_track->newTrack = decltype(playdate_sound_track->newTrack)(FUNC_TABLE_ADDRESS + 383 * 2 + 1);
	playdate_sound_track->freeTrack = decltype(playdate_sound_track->freeTrack)(FUNC_TABLE_ADDRESS + 384 * 2 + 1);
	playdate_sound_track->setInstrument = decltype(playdate_sound_track->setInstrument)(FUNC_TABLE_ADDRESS + 385 * 2 + 1);
	playdate_sound_track->getInstrument = decltype(playdate_sound_track->getInstrument)(FUNC_TABLE_ADDRESS + 386 * 2 + 1);
	playdate_sound_track->addNoteEvent = decltype(playdate_sound_track->addNoteEvent)(FUNC_TABLE_ADDRESS + 387 * 2 + 1);
	playdate_sound_track->removeNoteEvent = decltype(playdate_sound_track->removeNoteEvent)(FUNC_TABLE_ADDRESS + 388 * 2 + 1);
	playdate_sound_track->clearNotes = decltype(playdate_sound_track->clearNotes)(FUNC_TABLE_ADDRESS + 389 * 2 + 1);
	playdate_sound_track->getControlSignalCount = decltype(playdate_sound_track->getControlSignalCount)(FUNC_TABLE_ADDRESS + 390 * 2 + 1);
	playdate_sound_track->getControlSignal = decltype(playdate_sound_track->getControlSignal)(FUNC_TABLE_ADDRESS + 391 * 2 + 1);
	playdate_sound_track->clearControlEvents = decltype(playdate_sound_track->clearControlEvents)(FUNC_TABLE_ADDRESS + 392 * 2 + 1);
	playdate_sound_track->getPolyphony = decltype(playdate_sound_track->getPolyphony)(FUNC_TABLE_ADDRESS + 393 * 2 + 1);
	playdate_sound_track->activeVoiceCount = decltype(playdate_sound_track->activeVoiceCount)(FUNC_TABLE_ADDRESS + 394 * 2 + 1);
	playdate_sound_track->setMuted = decltype(playdate_sound_track->setMuted)(FUNC_TABLE_ADDRESS + 395 * 2 + 1);
	playdate_sound_track->getLength = decltype(playdate_sound_track->getLength)(FUNC_TABLE_ADDRESS + 396 * 2 + 1);
	playdate_sound_track->getIndexForStep = decltype(playdate_sound_track->getIndexForStep)(FUNC_TABLE_ADDRESS + 397 * 2 + 1);
	playdate_sound_track->getNoteAtIndex = decltype(playdate_sound_track->getNoteAtIndex)(FUNC_TABLE_ADDRESS + 398 * 2 + 1);
	playdate_sound_track->getSignalForController = decltype(playdate_sound_track->getSignalForController)(FUNC_TABLE_ADDRESS + 399 * 2 + 1);
	auto playdate_sound_instrument = (playdate_sound_instrument_32 *) ((uintptr_t) api + offset);
	playdate_sound->instrument = (decltype(playdate_sound->instrument)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_instrument_32);
	playdate_sound_instrument->newInstrument = decltype(playdate_sound_instrument->newInstrument)(FUNC_TABLE_ADDRESS + 400 * 2 + 1);
	playdate_sound_instrument->freeInstrument = decltype(playdate_sound_instrument->freeInstrument)(FUNC_TABLE_ADDRESS + 401 * 2 + 1);
	playdate_sound_instrument->addVoice = decltype(playdate_sound_instrument->addVoice)(FUNC_TABLE_ADDRESS + 402 * 2 + 1);
	playdate_sound_instrument->playNote = decltype(playdate_sound_instrument->playNote)(FUNC_TABLE_ADDRESS + 403 * 2 + 1);
	playdate_sound_instrument->playMIDINote = decltype(playdate_sound_instrument->playMIDINote)(FUNC_TABLE_ADDRESS + 404 * 2 + 1);
	playdate_sound_instrument->setPitchBend = decltype(playdate_sound_instrument->setPitchBend)(FUNC_TABLE_ADDRESS + 405 * 2 + 1);
	playdate_sound_instrument->setPitchBendRange = decltype(playdate_sound_instrument->setPitchBendRange)(FUNC_TABLE_ADDRESS + 406 * 2 + 1);
	playdate_sound_instrument->setTranspose = decltype(playdate_sound_instrument->setTranspose)(FUNC_TABLE_ADDRESS + 407 * 2 + 1);
	playdate_sound_instrument->noteOff = decltype(playdate_sound_instrument->noteOff)(FUNC_TABLE_ADDRESS + 408 * 2 + 1);
	playdate_sound_instrument->allNotesOff = decltype(playdate_sound_instrument->allNotesOff)(FUNC_TABLE_ADDRESS + 409 * 2 + 1);
	playdate_sound_instrument->setVolume = decltype(playdate_sound_instrument->setVolume)(FUNC_TABLE_ADDRESS + 410 * 2 + 1);
	playdate_sound_instrument->getVolume = decltype(playdate_sound_instrument->getVolume)(FUNC_TABLE_ADDRESS + 411 * 2 + 1);
	playdate_sound_instrument->activeVoiceCount = decltype(playdate_sound_instrument->activeVoiceCount)(FUNC_TABLE_ADDRESS + 412 * 2 + 1);
	auto playdate_sound_signal = (playdate_sound_signal_32 *) ((uintptr_t) api + offset);
	playdate_sound->signal = (decltype(playdate_sound->signal)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_sound_signal_32);
	playdate_sound_signal->newSignal = decltype(playdate_sound_signal->newSignal)(FUNC_TABLE_ADDRESS + 422 * 2 + 1);
	playdate_sound_signal->freeSignal = decltype(playdate_sound_signal->freeSignal)(FUNC_TABLE_ADDRESS + 423 * 2 + 1);
	playdate_sound_signal->getValue = decltype(playdate_sound_signal->getValue)(FUNC_TABLE_ADDRESS + 424 * 2 + 1);
	playdate_sound_signal->setValueScale = decltype(playdate_sound_signal->setValueScale)(FUNC_TABLE_ADDRESS + 425 * 2 + 1);
	playdate_sound_signal->setValueOffset = decltype(playdate_sound_signal->setValueOffset)(FUNC_TABLE_ADDRESS + 426 * 2 + 1);
	auto playdate_lua = (playdate_lua_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->lua = (decltype(PlaydateAPI->lua)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_lua_32);
	playdate_lua->addFunction = decltype(playdate_lua->addFunction)(FUNC_TABLE_ADDRESS + 427 * 2 + 1);
	playdate_lua->registerClass = decltype(playdate_lua->registerClass)(FUNC_TABLE_ADDRESS + 428 * 2 + 1);
	playdate_lua->pushFunction = decltype(playdate_lua->pushFunction)(FUNC_TABLE_ADDRESS + 429 * 2 + 1);
	playdate_lua->indexMetatable = decltype(playdate_lua->indexMetatable)(FUNC_TABLE_ADDRESS + 430 * 2 + 1);
	playdate_lua->stop = decltype(playdate_lua->stop)(FUNC_TABLE_ADDRESS + 431 * 2 + 1);
	playdate_lua->start = decltype(playdate_lua->start)(FUNC_TABLE_ADDRESS + 432 * 2 + 1);
	playdate_lua->getArgCount = decltype(playdate_lua->getArgCount)(FUNC_TABLE_ADDRESS + 433 * 2 + 1);
	playdate_lua->getArgType = decltype(playdate_lua->getArgType)(FUNC_TABLE_ADDRESS + 434 * 2 + 1);
	playdate_lua->argIsNil = decltype(playdate_lua->argIsNil)(FUNC_TABLE_ADDRESS + 435 * 2 + 1);
	playdate_lua->getArgBool = decltype(playdate_lua->getArgBool)(FUNC_TABLE_ADDRESS + 436 * 2 + 1);
	playdate_lua->getArgInt = decltype(playdate_lua->getArgInt)(FUNC_TABLE_ADDRESS + 437 * 2 + 1);
	playdate_lua->getArgFloat = decltype(playdate_lua->getArgFloat)(FUNC_TABLE_ADDRESS + 438 * 2 + 1);
	playdate_lua->getArgString = decltype(playdate_lua->getArgString)(FUNC_TABLE_ADDRESS + 439 * 2 + 1);
	playdate_lua->getArgBytes = decltype(playdate_lua->getArgBytes)(FUNC_TABLE_ADDRESS + 440 * 2 + 1);
	playdate_lua->getArgObject = decltype(playdate_lua->getArgObject)(FUNC_TABLE_ADDRESS + 441 * 2 + 1);
	playdate_lua->getBitmap = decltype(playdate_lua->getBitmap)(FUNC_TABLE_ADDRESS + 442 * 2 + 1);
	playdate_lua->getSprite = decltype(playdate_lua->getSprite)(FUNC_TABLE_ADDRESS + 443 * 2 + 1);
	playdate_lua->pushNil = decltype(playdate_lua->pushNil)(FUNC_TABLE_ADDRESS + 444 * 2 + 1);
	playdate_lua->pushBool = decltype(playdate_lua->pushBool)(FUNC_TABLE_ADDRESS + 445 * 2 + 1);
	playdate_lua->pushInt = decltype(playdate_lua->pushInt)(FUNC_TABLE_ADDRESS + 446 * 2 + 1);
	playdate_lua->pushFloat = decltype(playdate_lua->pushFloat)(FUNC_TABLE_ADDRESS + 447 * 2 + 1);
	playdate_lua->pushString = decltype(playdate_lua->pushString)(FUNC_TABLE_ADDRESS + 448 * 2 + 1);
	playdate_lua->pushBytes = decltype(playdate_lua->pushBytes)(FUNC_TABLE_ADDRESS + 449 * 2 + 1);
	playdate_lua->pushBitmap = decltype(playdate_lua->pushBitmap)(FUNC_TABLE_ADDRESS + 450 * 2 + 1);
	playdate_lua->pushSprite = decltype(playdate_lua->pushSprite)(FUNC_TABLE_ADDRESS + 451 * 2 + 1);
	playdate_lua->pushObject = decltype(playdate_lua->pushObject)(FUNC_TABLE_ADDRESS + 452 * 2 + 1);
	playdate_lua->retainObject = decltype(playdate_lua->retainObject)(FUNC_TABLE_ADDRESS + 453 * 2 + 1);
	playdate_lua->releaseObject = decltype(playdate_lua->releaseObject)(FUNC_TABLE_ADDRESS + 454 * 2 + 1);
	playdate_lua->setUserValue = decltype(playdate_lua->setUserValue)(FUNC_TABLE_ADDRESS + 455 * 2 + 1);
	playdate_lua->getUserValue = decltype(playdate_lua->getUserValue)(FUNC_TABLE_ADDRESS + 456 * 2 + 1);
	playdate_lua->callFunction_deprecated = decltype(playdate_lua->callFunction_deprecated)(FUNC_TABLE_ADDRESS + 457 * 2 + 1);
	playdate_lua->callFunction = decltype(playdate_lua->callFunction)(FUNC_TABLE_ADDRESS + 458 * 2 + 1);
	auto playdate_json = (playdate_json_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->json = (decltype(PlaydateAPI->json)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_json_32);
	playdate_json->initEncoder = decltype(playdate_json->initEncoder)(FUNC_TABLE_ADDRESS + 459 * 2 + 1);
	playdate_json->decode = decltype(playdate_json->decode)(FUNC_TABLE_ADDRESS + 460 * 2 + 1);
	playdate_json->decodeString = decltype(playdate_json->decodeString)(FUNC_TABLE_ADDRESS + 461 * 2 + 1);
	auto playdate_scoreboards = (playdate_scoreboards_32 *) ((uintptr_t) api + offset);
	PlaydateAPI->scoreboards = (decltype(PlaydateAPI->scoreboards)) (uintptr_t) (API_ADDRESS + offset);
	offset += sizeof(playdate_scoreboards_32);
	playdate_scoreboards->addScore = decltype(playdate_scoreboards->addScore)(FUNC_TABLE_ADDRESS + 462 * 2 + 1);
	playdate_scoreboards->getPersonalBest = decltype(playdate_scoreboards->getPersonalBest)(FUNC_TABLE_ADDRESS + 463 * 2 + 1);
	playdate_scoreboards->freeScore = decltype(playdate_scoreboards->freeScore)(FUNC_TABLE_ADDRESS + 464 * 2 + 1);
	playdate_scoreboards->getScoreboards = decltype(playdate_scoreboards->getScoreboards)(FUNC_TABLE_ADDRESS + 465 * 2 + 1);
	playdate_scoreboards->freeBoardsList = decltype(playdate_scoreboards->freeBoardsList)(FUNC_TABLE_ADDRESS + 466 * 2 + 1);
	playdate_scoreboards->getScores = decltype(playdate_scoreboards->getScores)(FUNC_TABLE_ADDRESS + 467 * 2 + 1);
	playdate_scoreboards->freeScoresList = decltype(playdate_scoreboards->freeScoresList)(FUNC_TABLE_ADDRESS + 468 * 2 + 1);
	auto json_encoder = (json_encoder_32 *) ((uintptr_t) api + offset);
	offset += sizeof(json_encoder_32);
	json_encoder->startArray = decltype(json_encoder->startArray)(FUNC_TABLE_ADDRESS + 469 * 2 + 1);
	json_encoder->addArrayMember = decltype(json_encoder->addArrayMember)(FUNC_TABLE_ADDRESS + 470 * 2 + 1);
	json_encoder->endArray = decltype(json_encoder->endArray)(FUNC_TABLE_ADDRESS + 471 * 2 + 1);
	json_encoder->startTable = decltype(json_encoder->startTable)(FUNC_TABLE_ADDRESS + 472 * 2 + 1);
	json_encoder->addTableMember = decltype(json_encoder->addTableMember)(FUNC_TABLE_ADDRESS + 473 * 2 + 1);
	json_encoder->endTable = decltype(json_encoder->endTable)(FUNC_TABLE_ADDRESS + 474 * 2 + 1);
	json_encoder->writeNull = decltype(json_encoder->writeNull)(FUNC_TABLE_ADDRESS + 475 * 2 + 1);
	json_encoder->writeFalse = decltype(json_encoder->writeFalse)(FUNC_TABLE_ADDRESS + 476 * 2 + 1);
	json_encoder->writeTrue = decltype(json_encoder->writeTrue)(FUNC_TABLE_ADDRESS + 477 * 2 + 1);
	json_encoder->writeInt = decltype(json_encoder->writeInt)(FUNC_TABLE_ADDRESS + 478 * 2 + 1);
	json_encoder->writeDouble = decltype(json_encoder->writeDouble)(FUNC_TABLE_ADDRESS + 479 * 2 + 1);
	json_encoder->writeString = decltype(json_encoder->writeString)(FUNC_TABLE_ADDRESS + 480 * 2 + 1);
	return offset;
}
