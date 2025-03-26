#include "thread.h"

unsigned long balance = 100;

void paypal_withdraw(int amt) {
    if (balance >= amt) {
        // Bugs may only manifest on specific timings. Sometimes
        // we reproduce bugs by inserting sleep()s.

        // usleep(1);

        balance -= amt;
    }
}

void T_paypal() {
    paypal_withdraw(100);
}

int main() {
    create(T_paypal);
    create(T_paypal);
    join();
    printf("balance = %lu\n", balance);
}
