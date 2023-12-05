#include "mac.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

void darkenTitlebar(void) {
	if (@available(macOS 10.14, *)) {
		NSAppearance *appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
		NSApplication *app = [NSApplication sharedApplication];
		[app setAppearance:appearance];
	}
}
