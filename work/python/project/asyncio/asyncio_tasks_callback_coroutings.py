import asyncio
 
@asyncio.coroutine
def my_coroutine(future, task_name, seconds_to_sleep=3):
    print('{0} sleeping for: {1} seconds'.format(task_name, seconds_to_sleep))
    yield from asyncio.sleep(seconds_to_sleep)
    future.set_result('{0} is finished'.format(task_name))
 
 
def got_result(future):
    print(future.result())
 
loop = asyncio.get_event_loop()
future1 = asyncio.Future()
future2 = asyncio.Future()
future3 = asyncio.Future()
 
tasks = [
    asyncio.ensure_future(my_coroutine(future1, 'task1', 3)),
    asyncio.ensure_future(my_coroutine(future2, 'task2', 1)),
    asyncio.ensure_future(my_coroutine(future3, 'task3', 2))]
 
future1.add_done_callback(got_result)
future2.add_done_callback(got_result)
future3.add_done_callback(got_result)
 
loop.run_until_complete(asyncio.gather(*tasks))
#loop.run_until_complete(asyncio.wait(tasks))
loop.close()
