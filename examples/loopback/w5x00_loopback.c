#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    // GPIO 22번 핀 설정
    const uint gpio_pin = 22;

    // 표준 입출력 초기화
    stdio_init_all();

    // GPIO 22번 핀을 입력 모드로 설정
    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, GPIO_IN);

    // 내부 풀업 활성화
    gpio_pull_up(gpio_pin);

    // 테스트 루프
    while (true) {
        // GPIO 상태 읽기
        bool pin_state = gpio_get(gpio_pin);

        // 핀 상태 출력
        printf("GPIO 22 state: %d\n", pin_state);

        // 500ms 대기
        sleep_ms(500);
    }

    return 0;
}