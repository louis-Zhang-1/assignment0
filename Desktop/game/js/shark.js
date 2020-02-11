var sharkObj = function(){
    this.x = [];
    this.y = [];
    this.alive = [];
    this.speed = [];
    this.sharkBody = new Image();
}


sharkObj.prototype.num = 5;

sharkObj.prototype.init = function(){
    for(let i = 0; i < this.num ; i++){
        this.x[i] = 10 + Math.random() * 100;
        this.y[i] = Math.random() * canHeight * 0.5;
        this.speed[i] = Math.random() + 0.5;
        this.alive[i] = true;
    }
    this.sharkBody.src = "./src/shark3.png";
}

sharkObj.prototype.draw = function(){
    for(let i = 0; i < this.num; i++){
        if(!data.gameOver){
            this.x[i] = this.x[i] + this.speed[i];
            this.speed[i] += gameTime * 0.0000005;
        }
        ctx1.save();   
        ctx1.translate(this.x[i],this.y[i]); 
        if(this.alive[i] == true){
            
            ctx1.drawImage(this.sharkBody, -this.sharkBody.width*0.5, -this.sharkBody.height*0.5);
            
        }
        ctx1.restore();
       

        if(this.x[i] > canWidth - 50){
            this.alive[i] = false;     
        }
    }

    for(var i = 0; i < shark.num; i++){
        if(!shark.alive[i]){
            shark.x[i] = 5 + Math.random() * 10;
            shark.y[i] = Math.random() * canHeight * 0.7;
            shark.alive[i] = true;
            this.speed[i] = Math.random() + 0.5;

        }
        
    }
}



// only send one dead shark

