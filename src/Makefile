CC=gcc
CXX=g++

CXXFLAGS=-std=c++17
CXXFLAGS+=-I/usr/include/libcamera -I/usr/include/EGL -I/usr/include/libdrm -I/usr/include/epoxy
CFLAGS=-I./lib

LIBS=-lpthread -lcamera -lcamera-base -lv4l2 -ldrm -lepoxy -lX11

OBJS=\
client.o \
main.o \
server.o \
camera/capture.o \
camera/h264_encoder.o \
config/config.o \
logger/logger.o \
preview/drm_preview.o \
preview/egl_preview.o \
preview/h264_decoder.o \
preview/null_preview.o \
preview/preview.o \
network/context.o \
network/decoder.o \
network/encoder.o \
network/socket_1.o \
network/socket_2.o \
network/socket_3.o \
network/queue/adapter.o \
network/queue/HPVTDecodeTimeManager.o \
network/queue/HPVTLimitter.o \
network/queue/HPVTPacket.o \
network/queue/HPVTPacketSet.o \
network/queue/HPVTPacketSetQueue.o \
network/queue/HPVTReceivePacketManager.o \
network/queue/HPVTTrafficRecoder.o \
network/queue/HPVTVideoFrame.o \
network/queue/HPVTVideoFrameQueue.o \
network/queue/HPVTVideoFrameBuilder.o \
network/queue/HPVTVideoFrameSequenceManager.o \
network/queue/notifier.o \
network/queue/transmitter.o \
network/queue/receiver.o \
lib/scs/1/socket.o \
lib/scs/1/time.o \
lib/scs/2/atomic.o \
lib/scs/2/logger.o \
lib/scs/2/pointer.o \
lib/scs/3/crc16.o \
lib/scs/3/worker.o \
lib/scs/4/gc.o \
lib/scs/5/callback/socket/feedback/config.o \
lib/scs/5/callback/socket/redundancy/config.o \
lib/scs/5/callback/socket/redundancy/functions.o \
lib/scs/5/callback/socket/redundancy/result.o \
lib/scs/5/callback/socket/redundancy/status.o \
lib/scs/5/callback/socket/types.o \
lib/scs/5/callback/system/action.o \
lib/scs/5/callback/system/error/config.o \
lib/scs/5/callback/system/error/status.o \
lib/scs/5/callback/system/list.o \
lib/scs/5/callback/system/socket/config.o \
lib/scs/5/callback/system/socket/status.o \
lib/scs/5/callback/system/system/config.o \
lib/scs/5/callback/system/system/status.o \
lib/scs/5/callback/system/types.o \
lib/scs/5/channel/channel.o \
lib/scs/5/channel/types.o \
lib/scs/5/context/caller.o \
lib/scs/5/context/closer.o \
lib/scs/5/context/context.o \
lib/scs/5/context/manager.o \
lib/scs/5/context/receiver.o \
lib/scs/5/context/sender.o \
lib/scs/5/feature/jitter.o \
lib/scs/5/feature/plr.o \
lib/scs/5/feature/pmtu.o \
lib/scs/5/feature/rtt.o \
lib/scs/5/feature/traffic/controller.o \
lib/scs/5/feature/traffic/counter.o \
lib/scs/5/feedback.o \
lib/scs/5/packet/builder.o \
lib/scs/5/packet/monitor.o \
lib/scs/5/packet/packet.o \
lib/scs/5/packet/parser.o \
lib/scs/5/packet/queue.o \
lib/scs/5/packet/sequencer.o \
lib/scs/5/packet/types.o \
lib/scs/5/packet/utils.o \
lib/scs/5/packet/v1/builder.o \
lib/scs/5/packet/v1/parser.o \
lib/scs/5/packet/watcher.o \
lib/scs/5/settings.o \
lib/scs/5/socket/datagram/settings.o \
lib/scs/5/socket/datagram/socket.o \
lib/scs/5/socket/queue.o \
lib/scs/5/socket/settings.o \
lib/scs/5/socket/socket.o \
lib/scs/5/socket/table.o \
lib/scs/5/socket/types.o \
lib/scs/5/socket/utils.o \
lib/scs/5/types.o \
lib/scs/6/fec/decoder.o \
lib/scs/6/fec/encoder.o \
lib/scs/6/fec/fec.o \
lib/scs/6/fec/types.o \
lib/scs/api.o \
lib/scs/internal.o \
lib/scs/types.o \

TARGET=hpvt

all:: $(TARGET)

clean:: 
	rm -f $(OBJS)

%.o: %.c
	@rm -f $@
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@rm -f $@
	$(CXX) $(CXXFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)
