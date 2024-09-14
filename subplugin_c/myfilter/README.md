gst-launch-1.0 rawdatagenerator header-value=0 idx-in-tensor=3 !  udpsink host=10.68.187.36 port=8000

# compare datareschedule and tensor_aggregator
# use datareschedule
gst-launch-1.0 udpsrc port=8000 ! datareschedule single-frame-size=102 accumulate-frame-number=3 ! printdata ! fakesink

# use tensor_aggregator
gst-launch-1.0 udpsrc port=8000 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=10:10:1:1,types=uint8 ! tensor_aggregator frames-in=1 frames-out=3 frames-flush=3 frames-dim=2 ! printdata ! fakesink

