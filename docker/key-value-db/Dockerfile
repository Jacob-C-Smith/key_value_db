# syntax=docker/dockerfile:1

FROM fedora

RUN dnf -y install clang make net-tools hostname 

WORKDIR /home
COPY ../../ /home/
EXPOSE 6713
RUN (cd /home/; make -C /home/gsdk)
RUN (cd /home/; make clean; make)
RUN (cd /home/; chmod +x docker/key-value-db/fork.sh)
CMD (cd /home/; ./docker/key-value-db/fork.sh)