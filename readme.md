# M5Stackで便利なWiFiセットアップ方法を整理
- SmartConfigという便利な設定の仕組みがArduino系のボードにはあるみたいなのでそれを活用。
- SPIFFSという機能も同じくArduino系のボードにある電源が切れても消えないメモリー領域なので、そこにWiFiの設定を書き込むようにする。
- したがって一般のArduino(ESP32)系ボードにも活用できる仕組みのはず