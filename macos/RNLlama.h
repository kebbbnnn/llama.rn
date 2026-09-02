#import <React/RCTBridgeModule.h>
#import <React/RCTBridge+Private.h>
#import <ReactCommon/RCTTurboModule.h>

// macOS host bridge module. Mirrors ios/RNLlama.h; the RN-macOS bridge APIs
// are identical to the iOS ones, so only the Metal availability logic in
// RNLlama.mm differs.
@interface RNLlama : NSObject <RCTBridgeModule>

@end