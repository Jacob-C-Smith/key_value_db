# syntax=docker/dockerfile:1

FROM fedora

RUN dnf -y install go clang make net-tools hostname 

WORKDIR /home
COPY . /home/
EXPOSE 3013
RUN (cd /home/; make -C /home/gsdk)
RUN (cd /home/; make clean; make)
RUN (cd /home/; chmod +x resources/fork.sh)
CMD (cd /home/; ./resources/fork.sh)