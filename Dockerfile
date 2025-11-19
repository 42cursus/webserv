FROM nginx:1.29

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    	fcgiwrap spawn-fcgi python3 ca-certificates \
    	htop iftop iproute2 procps && \
    rm -rf /var/lib/apt/lists/*

# fcgiwrap socket dir
RUN mkdir -p /run

# nginx config goes in here at build time or via a bind mount
# COPY resources/default.conf /etc/nginx/conf.d/default.conf

# tiny entrypoint to run fcgiwrap then nginx
COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
