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
ENV SDS_DIR=$HOME/sds

# install spack
RUN echo $INSTALL_DIR && mkdir -p $INSTALL_DIR
RUN git clone https://github.com/spack/spack ${SPACK_DIR}
RUN git clone https://github.com/hariharan-devarajan/hcl.git ${PROJECT_DIR}

ENV spack=${SPACK_DIR}/bin/spack
RUN . ${SPACK_DIR}/share/spack/setup-env.sh

# install software
ENV HCL_VERSION=dev

#RUN $spack spec "hcl@${HCL_VERSION}"
RUN echo 1
RUN cd ${PROJECT_DIR} && git fetch && git checkout feature/no_rpclib && git pull

RUN $spack repo add ${PROJECT_DIR}/dependency/hcl


COPY ./packages.yaml /root/.spack/packages.yaml

ENV HCL_SPEC=hcl@${HCL_VERSION}

RUN $spack install mochi-thallium@0.11.3~cereal ^mercury@2.3.1+ofi+ucx ^libfabric@1.23.1 fabrics=rxm,sockets,tcp ^ucx@1.15.1 boost@1.71.0


# COPY ./packages.yaml /root/.spack/packages.yaml
# RUN $spack external find

# RUN $spack install mpich@3.3.2

# ## Link Software
RUN $spack view symlink -i ${INSTALL_DIR} mpich@3.3.2 mochi-thallium@0.11.3~cereal ^mercury@2.3.1+ofi+ucx ^libfabric@1.23.1 fabrics=rxm,sockets,tcp ^ucx@1.15.1 boost@1.71.0

RUN echo "export PATH=${SPACK_ROOT}/bin:$PATH" >> /root/.bashrc
RUN echo ". $SPACK_ROOT/share/spack/setup-env.sh" >> /root/.bashrc

SHELL ["/bin/bash", "-c"]

