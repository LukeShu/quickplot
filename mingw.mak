all-before: config.h

config.h: mingw_config.h
	cp mingw_config.h config.h

