# node version 20 image

FROM node:20

# working diectory of the application 

WORKDIR /app

# copy all the files from here to the image

COPY . .

# command to install the packages on the image

RUN npm install

# adding the env variables
# ENV key=value
# ENV key1=value1
# ENV key2=value2

# expose the specific ports that you want to run on image that to are to mentioned in the application

EXPOSE 3000

# command to run the application

CMD ["node", "homato-control-system/app.js"]
