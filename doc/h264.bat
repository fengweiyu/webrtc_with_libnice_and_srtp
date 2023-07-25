::ffmpeg -i vv.mp4 -c copy test.h264
::-profile:v high -level:v 4.1 
::ffmpeg -i vv.mkv -acodec copy -vcodec libx264 -bsf: h264_mp4toannexb -profile:v main -level:v 4.1 -vf scale=1280:720 -vb 470k -f h264 test.h264
ffmpeg -i vv.mkv -acodec copy -vcodec libx264 -bsf: h264_mp4toannexb -profile:v baseline -level:v 3.1 -vf scale=1280:720 -vb 470k -f h264 test.h264
pause