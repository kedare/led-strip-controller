FROM node:latest
MAINTAINER Mathieu Poussin <mathieu.poussin@netyxia.net>


# Initialize
RUN mkdir -p /data/app
WORKDIR /data/app
COPY . /data/app

RUN npm install && \
    npm install -g pm2

EXPOSE 8080
CMD ["pm2", "start", "-x", "main.js", "--no-daemon"]
