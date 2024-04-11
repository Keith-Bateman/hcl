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
   openmpi-bin \
   libopenmpi-dev \
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
RUN git clone https://github.com/hariharan-devarajan/hcl.git ${PROJECT_DIR} && cd ${PROJECT_DIR} && git checkout feature/structure_changes

ENV spack=${SPACK_DIR}/bin/spack
RUN . ${SPACK_DIR}/share/spack/setup-env.sh
RUN $spack repo add ${PROJECT_DIR}/dependency/hcl

# install software
ENV HCL_VERSION=develop

COPY ./packages.yaml /root/.spack/packages.yaml
RUN $spack spec "hcl@${HCL_VERSION}"
ENV HCL_SPEC=hcl@${HCL_VERSION}
# RUN $spack install --only dependencies ${HCL_SPEC} +rpclib +thallium

# ## Link Software
# RUN $spack view symlink -i ${INSTALL_DIR} gcc@10.3.0 rpclib@2.2.1 mochi-thallium@0.11.3 boost@1.71.0

# RUN echo "export PATH=${SPACK_ROOT}/bin:$PATH" >> /root/.bashrc
# RUN echo ". $SPACK_ROOT/share/spack/setup-env.sh" >> /root/.bashrc

SHELL ["/bin/bash", "-c"]