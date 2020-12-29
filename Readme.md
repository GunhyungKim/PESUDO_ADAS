C++로 작성한 종프 소스코드

환경
   
    Nvidia Jetson TX2
    
    Ubuntu
    
    Stereo Camera (해당 Project에서는 oCam)

    
외부라이브러리

    소켓관련

    https://github.com/zaphoyd/websocketpp - 사용중. git clone 명령으로 복사후, cd websocketpp, mkdir build, cd build, cmake .., sudo make install로 설치.


빌드전 준비할것

    1. 소켓 관련 라이브러리 설치 (주소: https://github.com/zaphoyd/websocketpp)
    2. preinstall.sh 실행
    3. darknet 빌드 및 libdarknet.so 파일을 cpp_project폴더로 복사 (darknet 빌드시 makefile에서 opencv, cuda, cudnn 사용옵션 on)
    4. dataset_test 레포지토리 복사 (darknet에 사용되는 cfg파일 때문)
    5. tony-yolo3 딥러닝시킨 파일 다운로드 및 darknet/backup1/ 폴더에 복사. (다운링크: https://drive.google.com/file/d/1YmjEJE0WOcNSOxyxe8_3KJPuuz6aI6QA/view?usp=sharing)

빌드 방법

    cpp_project 폴더에서 make 실행


실행 방법

    스테레오 이미지 처리시 oCam 필요
    ./main stereo 로 스테레오 실행
    ./main video [file_name] 로 비디오 파일 처리

stereo 처리 관련

    미리 준비해둔 calibation 파일(cali.yaml)을 사용하지 않고 새로 calibration 진행을 원할시 calibration 이미지 필요. 
    calibration 이미지는 기본적으로 ../darknet/python/stereo/imgs 폴더에서 찾도록 되어 있다.
    다른 경로를 사용하고자 하는 경우에는 stereo.cpp에서 경로를 수정해준다.
    
    캘리브레이션 이미지를 저장하는 코드는 c++로 아직 만들지 않았으르로 기존의 python 코드를 사용하면됨


        
