# syntax=docker/dockerfile:1

FROM fedora

RUN dnf -y install go make net-tools hostname 

WORKDIR /home
COPY ../../ /home/
EXPOSE 3013
RUN (cd /home/; chmod +x docker/key-value-db-http/fork.sh)
CMD (cd /home/; ./docker/key-value-db-http/fork.sh)