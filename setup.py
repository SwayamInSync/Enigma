from setuptools import setup, find_packages

setup(
    name="enigma",
    version="0.0.1",
    packages=find_packages(),
    install_requires=[],
    author="Swayam Singh",
    author_email="singhswayam008@gmail.com",
    description="Python bindings for the Enigma tensor framework",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/swayaminsync/enigma",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.8",
)
