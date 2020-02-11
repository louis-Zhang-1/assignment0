var fruitObj  = function(){
    this.x = [];
    this.y = [];
    // the length of picture
    this.l = [];
    this.alive = [];
    // the move speed of the fruits
    this.spd = [];
    this.fruitType = [];
    this.orange = new Image();
    this.blue = new Image();
}

fruitObj.prototype.num = 35;
fruitObj.prototype.init = function(){
    for( var i = 0; i < this.num; i++){
        this.alive[i] = false;
        this.x[i] = 0;
        this.y[i] = 0;
        this.spd[i] = Math.random() * 0.01 + 0.005;
        // the size of the fruit
        this.l[i] = 0;
        this.fruitType[i] = "";
        this.born(i);   
    }
    this.orange.src = "./src/fruit.png";
    this.blue.src = "./src/blue.png";
}  

fruitObj.prototype.draw = function(){
    for(var i = 0; i < this.num; i++){
        // deltaTime: the time between two frame
        
        if(this.alive[i] == true){
            if(this.fruitType[i] == "blue"){
                var pic = this.blue;
            }else{
                var pic = this.orange;
            }
            // get larger with the time
            if(this.l[i] <= 15){
                this.l[i] += this.spd[i] * deltaTime;
            }else{
                this.y[i] -= this.spd[i] * 5 * deltaTime;
            }
            // the last two parameters means the size of the image
            ctx2.drawImage(pic , this.x[i], this.y[i], this.l[i], this.l[i]);
            if(this.y[i] < - 10){
                this.alive[i] = false;
            } 
        }
    }
}

//control the numbers of fruits to no less than 15
function fruitMoniter(){
    var num = 0;
    for(var i = 0; i < fruit.num; i++){
        if(fruit.alive[i]){
            num++;
        }
        if(num < 13){
            sendFruit();
            // send fruit
            return;
        }
    }
}

// only send one dead fruit
function sendFruit(){
    for(var i = 0; i < fruit.num ; i++){
        if(!fruit.alive[i]){
            fruit.born(i);
            return;
        }
    }
}

fruitObj.prototype.born = function(i){
    var aneID = Math.floor(Math.random() * ane.num);
    this.x[i] = ane.x[aneID];
    this.y[i] = canHeight - ane.len[aneID];
    this.l[i] = 0;
    this.alive[i] = true;
    var ran = Math.random();
    if(ran < 0.4){
        this.fruitType[i] = "blue";
    }else{
        this.fruitType[i] = "orange";
    }
}

fruitObj.prototype.dead = function(i) {
    this.alive[i] = false;
}

