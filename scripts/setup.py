from distutils.core import setup

setup(
    name="congestion-runner",
    version="0.1dev",
    packages=['congestion_runner'],
    install_requires=[
        'click', 'toml'
    ],
    entry_points='''
        [console_scripts]
        run=congestion_runner.run:main
    '''
    )
