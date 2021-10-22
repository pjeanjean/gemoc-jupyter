FROM condaforge/mambaforge:latest

ENV DEBIAN_FRONTEND noninteractive

COPY . /root/gemoc-xeus
WORKDIR /root/gemoc-xeus

RUN mamba install -c conda-forge\
	cmake\
  	cppzmq\
	libuuid\
	nlohmann_json\
	OpenSSL\
  	pkg-config\
	xeus\
	xtl\
	zeromq

RUN apt-get -y update && apt-get -y install build-essential

RUN mkdir build && cd build && cmake .. && make

RUN rm -rf /var/lib/{apt,dpkg,cache,log}

ENTRYPOINT [ "/root/gemoc-xeus/build/gemoc_kernel" ]
