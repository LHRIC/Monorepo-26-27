# syntax=docker/dockerfile:1
FROM python:3.12-slim AS base
ENV PYTHONDONTWRITEBYTECODE=1 PYTHONUNBUFFERED=1
WORKDIR /opt/car-sandbox
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
RUN groupadd --gid 10001 recruit && useradd --uid 10001 --gid 10001 --no-create-home --shell /usr/sbin/nologin recruit \
    && mkdir -p /home/recruit /state /opt/prize \
    && chown -R 10001:10001 /home/recruit /state
COPY sandbox ./sandbox
# Docker copies this home directory into the named volume on its first use.
COPY --chown=10001:10001 challenge /home/recruit/challenge
COPY --chown=10001:10001 challenge/commands.txt /home/recruit/commands.txt
COPY prize/datasheet.txt /opt/prize/datasheet.txt
USER 10001:10001

FROM base AS test
USER root
RUN apt-get update && apt-get install -y --no-install-recommends openssh-client \
    && rm -rf /var/lib/apt/lists/*
COPY tests ./tests
COPY challenge ./challenge
COPY prize ./prize
USER 10001:10001
RUN python -m unittest discover -s tests -v

FROM base AS runtime
EXPOSE 2222
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 CMD ["python", "-m", "sandbox.health"]
CMD ["python", "-m", "sandbox.server"]
