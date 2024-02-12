#include "mac.h"

#ifdef __APPLE__
#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
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

void getScreenDPI(float *horizontalDPI, float *verticalDPI) {
#ifdef __APPLE__
	NSScreen *screen = [NSScreen mainScreen];
	NSDictionary *screenInfo = [screen deviceDescription];
	NSSize screenSize = [[screenInfo objectForKey:NSDeviceSize] sizeValue];
	NSRect screenFrame = [screen frame];

	// Calculate the horizontal and vertical DPI
	*horizontalDPI = screenSize.width / (NSWidth(screenFrame) / 72.0);
	*verticalDPI = screenSize.height / (NSHeight(screenFrame) / 72.0);
#endif
}
