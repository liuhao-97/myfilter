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





gst-launch-1.0 rawdatagenerator datasize=100  header-value=0 idx-in-tensor=25 ifseq=1  ! udpsink host=10.68.186.155 port=6001



gst-launch-1.0 rawdatagenerator datasize=100  header-value=0 idx-in-tensor=25 ifseq=1  ! printdata headtail=0   ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=5:5:1:1, types=float32 !  tensor_transform mode=transpose option=1:2:0:3  ! printdata headtail=0 ! fakesink




# data check pair pass using splitaddhead
gst-launch-1.0 rawdatagenerator datasize=1044480  header-value=0 idx-in-tensor=1 ifseq=1 start=22 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=32:32:255:1,types=float32 ! splitaddhead single-frame-size=61440 accumulate-frame-number=17 !  udpsink host=10.68.187.36 port=8000

gst-launch-1.0 udpsrc port=8000 ! queue  ! printdata headtail=20 ! datareschedule single-frame-size=61442 accumulate-frame-number=17 !   datacheck datasize=1044480  header-value=0 idx-in-tensor=1 ifseq=1 start=22 large-tensor-datasize=1044480 small-tensor-datasize=0 ! other/tensors,format=static,num_tensors=1,dimensions=32:32:255:1,types=float32,framerate=0/1 !  tensor_sink





# data check success
# sender
gst-launch-1.0 rawdatagenerator datasize=1048576  header-value=0 idx-in-tensor=1 ifseq=1 start=22 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=32:32:256:1,types=float32 ! tensor_split name=split tensorseg=32:32:255:1,32:32:1:1 split.src_0 ! queue ! splitaddhead single-frame-size=61440 accumulate-frame-number=17 ! udpsink host=10.68.187.36 port=8000  split.src_1 ! queue ! udpsink host=10.68.187.36 port=8001

# checker
gst-launch-1.0 udpsrc port=8000 ! queue ! printdata headtail=20 ! datareschedule single-frame-size=61442 accumulate-frame-number=17  ! other/tensors,format=static,num_tensors=1,dimensions=32:32:255:1,types=float32,framerate=0/1 ! merge.sink_0 
udpsrc port=8001 ! queue ! printdata headtail=20  ! other/tensors,format=static,num_tensors=1,dimensions=32:32:1:1,types=float32,framerate=0/1 ! merge.sink_1 
tensor_merge name=merge mode=linear option=2 ! other/tensors,format=static,framerate=0/1,num_tensors=1,dimensions=32:32:256:1,types=float32 !  datacheck datasize=1048576  header-value=0 idx-in-tensor=1 ifseq=1 large-tensor-datasize=1048576 small-tensor-datasize=0 start=22 ! fakesink




gst-launch-1.0 udpsrc port=8000 ! queue ! printdata headtail=0  ! datareschedule single-frame-size=102 accumulate-frame-number=1 ! printdata headtail=0  !  other/tensors,format=static,num_tensors=1,dimensions=5:5:1:1,types=float32,framerate=0/1 ! queue ! splitaddhead single-frame-size=100 accumulate-frame-number=1 !  printdata headtail=0  ! udpsink host=10.68.186.155 port=6666