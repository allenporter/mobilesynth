//
//  mobilesynthViewController.h
//  mobilesynth
//
//  Created by Allen Porter on 12/7/08.
//  Copyright thebends 2008. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AudioOutput.h"
#import "KeyboardView.h"
#import <AudioToolbox/AudioConverter.h>

namespace synth { class Controller; }
namespace synth { class Envelope; }
namespace synth { class LFO; }
namespace synth { class Oscillator; }
namespace synth { class Note; }
namespace synth { class LowPass; }

@class OscillatorView;
@class OscillatorDetailView;
@class ModulationView;
@class EnvelopeView;
@class FilterEnvelopeView;
@class FilterView;

@interface mobilesynthViewController : UIViewController <KeyboardDelegate, SampleGenerator> {
 @private
  UIScrollView* keyboardScrollView;
  KeyboardView* keyboardImageView;
  UIScrollView* controlScrollView;
  UIPageControl* controlPageControl;
        
  OscillatorView* oscillatorView;
  OscillatorDetailView* oscillatorDetailView;
  ModulationView* modulationView;
  FilterView* filterView;
  EnvelopeView* envelopeView;
  EnvelopeView* filterEnvelopeView;
  
  // Synthesizer components
  AudioOutput* output;
  synth::Controller* controller_;
  
  AudioStreamBasicDescription outputFormat;
  
  // Used to prevent a feedback loop
  BOOL pageControlUsed;
}

@property (nonatomic, retain) IBOutlet UIScrollView *keyboardScrollView;
@property (nonatomic, retain) IBOutlet KeyboardView *keyboardImageView;
@property (nonatomic, retain) IBOutlet UIScrollView *controlScrollView;
@property (nonatomic, retain) IBOutlet UIPageControl *controlPageControl;

@property (nonatomic, retain) IBOutlet OscillatorView *oscillatorView;
@property (nonatomic, retain) IBOutlet OscillatorDetailView *oscillatorDetailView;
@property (nonatomic, retain) IBOutlet ModulationView *modulationView;
@property (nonatomic, retain) IBOutlet FilterView *filterView;
@property (nonatomic, retain) IBOutlet EnvelopeView *envelopeView;
@property (nonatomic, retain) IBOutlet EnvelopeView *filterEnvelopeView;

- (void)noteBegin:(int)note;
- (void)noteEnd;
- (OSStatus)generateSamples:(AudioBufferList*)buffers;

// For control panel
- (IBAction)changePage:(id)sender;

@end

