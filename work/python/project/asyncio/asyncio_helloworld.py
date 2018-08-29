import asyncio

import datetime

def display_date(end_time, loop):
    print(datetime.datetime.now())
    if (loop.time() + 1.0) < end_time:
        loop.call_later(1, display_date, end_time, loop)
    else:
        loop.stop()

if __name__ == "__main__":
    loop = asyncio.get_event_loop()

    end_time = loop.time() + 5.0
    # Schedule a call to hello_world()
    loop.call_soon(display_date, end_time, loop)

    # Blocking call interrupted by loop.stop()
    loop.run_forever()
    loop.close()
