#include <stdio.h>
#include <unistd.h> /* sleep */

#include "airtime.h"
#if GLUON
static const char const *wifi_0_dev = "client0";
static const char const *wifi_1_dev = "client1";

#else
static const char const *wifi_0_dev = "wlan0";
static const char const *wifi_1_dev = "wlan1";

#endif /* GLUON */

void print_result(struct airtime_result *);

int main() {
	struct airtime *a;

	while (1) {
		a = get_airtime(wifi_0_dev,wifi_1_dev);
		print_result(&a->radio0);
		print_result(&a->radio1);
		sleep(1);
	}
}

void print_result(struct airtime_result *result){
	printf("freq=%d\tnoise=%d\tbusy=%lld\tactive=%lld\trx=%lld\ttx=%lld\n",
		result->frequency,
		result->noise,
		result->busy_time.current,
		result->active_time.current,
		result->rx_time.current,
		result->tx_time.current
	);
}
