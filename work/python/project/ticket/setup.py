from setuptools import setup

setup(
        name='tickets',
        version = '1.0',
        description = 'Get tickets info via 12306',
        author = 'wenxu',
        url = 'https://github.com/liuny05/tickets_12306/',
        py_modules=['tickets', 'stations', 'get_request', 'formats'],
        install_requires=['requests', 'docopt'],
        entry_points={
                    'console_scripts': ['tickets=tickets:cli']
                }
)
