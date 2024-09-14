gst-launch-1.0 rawdatagenerator header-value=0 idx-in-tensor=3 !  udpsink host=10.68.187.36 port=8000

# compare datareschedule and tensor_aggregator
# use datareschedule
gst-launch-1.0 udpsrc port=8000 ! datareschedule single-frame-size=102 accumulate-frame-number=3 ! printdata ! fakesink

# use tensor_aggregator
gst-launch-1.0 udpsrc port=8000 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=10:10:1:1,types=uint8 ! tensor_aggregator frames-in=1 frames-out=3 frames-flush=3 frames-dim=2 ! printdata ! fakesink



gst-launch-1.0 rawdatagenerator header-value=0 idx-in-tensor=1 !  udpsink host=10.68.187.36 port=8000

gst-launch-1.0 udpsrc port=8000 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=16:16:32:1,types=float32 ! tensor_aggregator frames-in=1 frames-out=2 frames-flush=2 frames-dim=2 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=16:16:64:1,types=float32 ! tensor_filter framework=pytorch model=../encoder_pos2_inch64_outch2_hw16.torchscript.pt input=16:16:64:1 inputtype=float32 inputname=input output=16:16:2:1 outputtype=float32 accelerator=true:gpu ! printdata ! tensor_sink


gst-launch-1.0 rawdatagenerator header-value=2 idx-in-tensor=2 !  udpsink host=10.68.187.36 port=8000

gst-launch-1.0 udpsrc port=8000 ! datareschedule single-frame-size=32770 accumulate-frame-number=2 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=16:16:64:1,types=float32 ! tensor_filter framework=pytorch model=../encoder_pos2_inch64_outch2_hw16.torchscript.pt input=16:16:64:1 inputtype=float32 inputname=input output=16:16:2:1 outputtype=float32 accelerator=true:gpu ! printdata ! tensor_sink
