sudo docker build -t app .
sudo docker rm -f appc
sudo docker run -d -it -p 3000:3000 --name appc app
