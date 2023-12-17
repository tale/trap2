#include "mac.h"

#ifdef __APPLE__
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#endif

void darkenTitlebar(void) {
#ifdef __APPLE__
	if (@available(macOS 10.14, *)) {
		NSAppearance *appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
		NSApplication *app = [NSApplication sharedApplication];
		[app setAppearance:appearance];
	}
#endif
}
