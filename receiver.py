#!/usr/bin/env python3

import os

import asyncio
import websockets

from datetime import datetime

def hide():
    os.system('amixer -D pulse sset Master mute')
    os.system('xdotool key XF86AudioPlay')
    with os.popen('wmctrl -l') as wmctl:
        for win_id in [x.split(' ')[0] for x in filter(lambda x: 'YouTube' in x, wmctl.read().split('\n'))]:
            os.system('xdotool windowactivate %s' % win_id)
            os.system('xdotool type " "')
            os.system('xdotool windowminimize %s' % win_id)

async def handle_data(websocket, path):
    print('connected')
    last_run = datetime.now()
    async for message in websocket:
        val = max([int(x) for x in message.split(',')])
        now = datetime.now()
        diff = (now - last_run).total_seconds()
        print(now, diff, val)
        if val > 1600 and diff > 10:
            last_run = datetime.now()
            hide()

if __name__ == "__main__":
    start_server = websockets.serve(handle_data, "0.0.0.0", 4457)

    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()

