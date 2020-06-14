#pragma once

#define MAKE_PLUGIN_VERSION(major, minor, rev)		(((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | (rev & 0xFFFF))

#define GET_PLUGIN_VERSION_MAJOR(a)					(((a) & 0xFF000000) >> 24)
#define GET_PLUGIN_VERSION_MINOR(a)					(((a) & 0x00FF0000) >> 16)
#define GET_PLUGIN_VERSION_REV(a)					(((a) & 0x0000FFFF) >> 0)
