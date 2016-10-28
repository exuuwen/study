make
gcc -o event_test event_test.c
gcc -o event_send_test  event_send_test.c


rmmod dvb_event
insmod dvb_event.ko


