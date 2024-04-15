FROM ubuntu:20.04

# general environment for docker
ENV        DEBIAN_FRONTEND=noninteractive \
   SPACK_ROOT=/root/spack \
   FORCE_UNSAFE_CONFIGURE=1

# install minimal spack dependencies
RUN        apt-get update \
   && apt-get install -y --no-install-recommends \
   autoconf \
   git \
   automake \
   libarchive-dev \
   lua5.3 liblua5.3-dev \
   python3 python3-pip \
   gcc g++ \
   mpich \
   hwloc \
   cmake pkg-config \
   libboost-all-dev \
   libtool libtool-bin \
   libfabric-dev libfabric-bin \
   libczmq-dev \
   lua-posix-dev \
   lz4 \
   libzmq5 \
   sqlite \
   make

# setup paths
ENV HOME=/root
ENV PROJECT_DIR=$HOME/source
ENV INSTALL_DIR=$HOME/install
ENV SPACK_DIR=$HOME/spack

# install spack
RUN echo $INSTALL_DIR && mkdir -p $INSTALL_DIR
RUN git clone https://github.com/spack/spack ${SPACK_DIR}

ENV spack=${SPACK_DIR}/bin/spack
RUN . ${SPACK_DIR}/share/spack/setup-env.sh

RUN python3 -m pip install cmake ninja pybind11  setuptools wheel

RUN mkdir -p ${INSTALL_DIR}
COPY ./spack.yaml ${INSTALL_DIR}/spack.yaml

RUN   . ${SPACK_DIR}/share/spack/setup-env.sh && \
   spack env activate -p ${INSTALL_DIR} && \
   spack install

RUN echo "export PATH=${SPACK_ROOT}/bin:$PATH" >> /root/.bashrc
RUN echo ". $SPACK_ROOT/share/spack/setup-env.sh" >> /root/.bashrc

SHELL ["/bin/bash", "-c"]

