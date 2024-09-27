gst-launch-1.0  udpsrc port=8887 ! splitaddheadc single-frame-size=2000 accumulate-frame-number=5 ! udpsink host=10.68.186.15
5 port=8888

gst-launch-1.0 fastrawdatagenerator udpsrc port=8887 ! splitaddheadc single-frame-size=2000 accumulate-frame-number=5 ! udpsi
nk host=10.68.186.155 port=8888
