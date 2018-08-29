import asyncio
import datetime


@asyncio.coroutine
def in_wait(seconds):
    print("in the wait")
    yield from asyncio.sleep(1)


@asyncio.coroutine
def display_date(loop):
    end_time = loop.time() + 5.0
    while True:
        print(datetime.datetime.now())
        if (loop.time() + 1.0) >= end_time:
            break;
        yield from in_wait(1)

if __name__ == "__main__":
    loop = asyncio.get_event_loop()

    # Schedule a call to hello_world()
    loop.run_until_complete(display_date(loop))

    loop.close()
