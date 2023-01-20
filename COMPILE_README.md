Der Compiler benÃtigt python2, damit es mit python3 auch klappt, mÃ¼ssen die ESP32-Projektfiles gÃndert werden:

    sed -i -e 's/=python /=python3 /g' ~/.arduino15/packages/esp32/hardware/esp32/*/platform.txt
