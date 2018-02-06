import os
import sys

if len(sys.argv) != 2:
    print('You should provide exactly one argument for video dir:\n%s video-path'%sys.argv[0])
    sys.exit(1)
video_files=os.listdir(sys.argv[1])
commands=['./hugeEvaluation.sh %s >>stdout.log 2>>stderr.log'%os.path.join(sys.argv[1],fname) for fname in video_files]

for c in commands:
    print(c)
    os.system(c)
