//
//  TPCircularBuffer+AudioBufferList.h
//  Circular/Ring buffer implementation
//
//  Created by Michael Tyson on 20/03/2012.
//  Copyright 2012 A Tasty Pixel. All rights reserved.
//

#ifndef TPCircularBuffer_AudioBufferList_h
#define TPCircularBuffer_AudioBufferList_h

#ifdef __cplusplus
extern "C" {
#endif

#include "TPCircularBuffer.h"
#include <AudioToolbox/AudioToolbox.h>
#include <pthread.h>
    
/*!
 * Prepare an empty buffer list, stored on the circular buffer
 *
 * @param buffer            Circular buffer
 * @param numberOfBuffers   The number of buffers to be contained within the buffer list
 * @param bytesPerBuffer    The number of bytes to store for each buffer
 * @param timestamp         The timestamp associated with the buffer, or NULL
 * @return The empty buffer list, or NULL if circular buffer has insufficient space
 */
AudioBufferList *TPCircularBufferPrepareEmptyAudioBufferList(TPCircularBuffer *buffer, int numberOfBuffers, int bytesPerBuffer, const AudioTimeStamp *timestamp);

/*!
 * Mark next audio buffer list as ready for reading
 *
 *  This marks the audio buffer list prepared using TPCircularBufferPrepareEmptyAudioBufferList
 *  as ready for reading. You must not call this function without first calling
 *  TPCircularBufferPrepareEmptyAudioBufferList.
 *
 * @param buffer            Circular buffer
 */
void TPCircularBufferProduceAudioBufferList(TPCircularBuffer *buffer);

/*!
 * Copy the audio buffer list onto the buffer, with a frame count limit
 *
 * @param buffer            Circular buffer
 * @param bufferList        Buffer list containing audio to copy to buffer
 * @param timestamp         The timestamp associated with the buffer, or NULL
 * @param frames            Number of frames of original buffer to copy
 * @param audioFormat The AudioStreamBasicDescription describing the audio
 * @return YES if buffer list was successfully copied; NO if there was insufficient space
 */
bool TPCircularBufferCopyAudioBufferListPartial(TPCircularBuffer *buffer, const AudioBufferList *bufferList, const AudioTimeStamp *timestamp, UInt32 frames, AudioStreamBasicDescription *audioFormat);

/*!
 * Copy the audio buffer list onto the buffer
 *
 * @param buffer            Circular buffer
 * @param bufferList        Buffer list containing audio to copy to buffer
 * @param timestamp         The timestamp associated with the buffer, or NULL
 * @return YES if buffer list was successfully copied; NO if there was insufficient space
 */
static __inline__ __attribute__((always_inline)) bool TPCircularBufferCopyAudioBufferList(TPCircularBuffer *buffer, const AudioBufferList *bufferList, const AudioTimeStamp *timestamp) {
    return TPCircularBufferCopyAudioBufferListPartial(buffer, bufferList, timestamp, UINT32_MAX, NULL);
}
    
/*!
 * Get a pointer to the next stored buffer list
 *
 * @param buffer            Circular buffer
 * @param outTimestamp      On output, if not NULL, the timestamp corresponding to the buffer
 * @return Pointer to the next buffer list in the buffer
 */
static __inline__ __attribute__((always_inline)) AudioBufferList *TPCircularBufferNextBufferList(TPCircularBuffer *buffer, AudioTimeStamp *outTimestamp) {
    int32_t dontcare; // Length of segment is contained within buffer list, so we can ignore this
    AudioTimeStamp *timestamp = (AudioTimeStamp *)TPCircularBufferTail(buffer, &dontcare);
    if ( !timestamp ) return NULL;
    if ( outTimestamp ) {
        *outTimestamp = *timestamp;
    }
    return (AudioBufferList*)(((char*)timestamp)+sizeof(AudioTimeStamp)+sizeof(UInt32));
}

/*!
 * Get a pointer to the next stored buffer list after the given one
 *
 * @param buffer            Circular buffer
 * @param bufferList        Preceding buffer list
 * @param outTimestamp      On output, if not NULL, the timestamp corresponding to the buffer
 * @return Pointer to the next buffer list in the buffer, or NULL
 */
AudioBufferList *TPCircularBufferNextBufferListAfter(TPCircularBuffer *buffer, AudioBufferList *bufferList, AudioTimeStamp *outTimestamp);

/*!
 * Consume the next buffer list
 *
 * @param buffer Circular buffer
 */
static __inline__ __attribute__((always_inline)) void TPCircularBufferConsumeNextBufferList(TPCircularBuffer *buffer) {
    int32_t dontcare;
    AudioTimeStamp *timestamp = (AudioTimeStamp *)TPCircularBufferTail(buffer, &dontcare);
    if ( !timestamp ) return;
    UInt32 *totalLengthInBytes = (UInt32*)(timestamp+1);
    TPCircularBufferConsume(buffer, sizeof(AudioTimeStamp)+sizeof(UInt32)+*totalLengthInBytes);
}

/*!
 * Consume a portion of the next buffer list
 *
 *  This will also increment the sample time and host time portions of the timestamp of
 *  the buffer list, if present.
 *
 * @param buffer Circular buffer
 * @param framesToConsume The number of frames to consume from the buffer list
 * @param audioFormat The AudioStreamBasicDescription describing the audio
 */
void TPCircularBufferConsumeNextBufferListPartial(TPCircularBuffer *buffer, int framesToConsume, AudioStreamBasicDescription *audioFormat);

/*!
 * Consume a certain number of frames from the buffer, possibly from multiple queued buffer lists
 *
 *  Copies the given number of frames from the buffer into outputBufferList, of the
 *  given audio description, then consumes the audio buffers. If an audio buffer has
 *  not been entirely consumed, then updates the queued buffer list structure to point
 *  to the unconsumed data only.
 *
 * @param buffer            Circular buffer
 * @param ioLengthInFrames  On input, the number of frames in the given audio format to consume; on output, the number of frames provided
 * @param outputBufferList  The buffer list to copy audio to, or NULL to discard audio. If not NULL, the structure must be initialised properly, and the mData pointers must not be NULL.
 * @param outTimestamp      On output, if not NULL, the timestamp corresponding to the first audio frame returned
 * @param audioFormat       The format of the audio stored in the buffer
 */
void TPCircularBufferDequeueBufferListFrames(TPCircularBuffer *buffer, UInt32 *ioLengthInFrames, AudioBufferList *outputBufferList, AudioTimeStamp *outTimestamp, AudioStreamBasicDescription *audioFormat);

/*!
 * Determine how many frames of audio are buffered
 *
 *  Given the provided audio format, determines the frame count of all queued buffers
 *
 * @param buffer            Circular buffer
 * @param outTimestamp      On output, if not NULL, the timestamp corresponding to the first audio frame returned
 * @param audioFormat       The format of the audio stored in the buffer
 * @return The number of frames in the given audio format that are in the buffer
 */
UInt32 TPCircularBufferPeek(TPCircularBuffer *buffer, AudioTimeStamp *outTimestamp, AudioStreamBasicDescription *audioFormat);
 
#ifdef __cplusplus
}
#endif

#endif
