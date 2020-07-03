set -x

docker build -t smodels:latest .

docker run --rm -it -v $(pwd):/app smodels:latest
