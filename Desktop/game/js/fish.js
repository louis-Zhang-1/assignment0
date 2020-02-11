var fishObj = function(){
    this.x;
    this.y;
    this.health;

    this.angle;
    this.bigBody = new Image();
}

fishObj.prototype.init = function(){
    this.health = 100;
    this.x = canWidth * 0.4;
    this.y = canHeight * 0.4;
    this.angle = 0;
    this.bigBody.src = "./src/fish.png";
 
    
}

fishObj.prototype.draw = function(){
  

    
    if(this.health <= 0){
        this.health = 0;
        data.gameOver = true;
    }else{
        if(!data.gameOver){
            this.health -= Math.log(gameTime + 1) * 0.005;
        }
    }
   
    this.x = lerpDistance(mx, this.x, 0.99);
    this.y = lerpDistance(my, this.y, 0.99);

    var deltaX = mx - this.x;
    var deltaY = my - this.y;
    var beta = Math.atan2(deltaY, deltaX) + Math.PI;
    this.angle = lerpAngle(beta, this.angle, 0.6);    

    ctx1.save();   
    ctx1.translate(this.x,this.y); 
    ctx1.rotate(this.angle);
    ctx1.drawImage(this.bigBody, -this.bigBody.width*0.5, -this.bigBody.height*0.5); 
    ctx1.restore();
    
}

function lerpDistance(aim, cur, ratio) {
	var delta = cur - aim;
	return aim + delta * ratio;
}


function lerpAngle(a, b, t) {
	var d = b - a;
	if (d > Math.PI){ 
        d = d -  2 * Math.PI;
    }
	if (d < -Math.PI) {
        d = d +  2 * Math.PI;
    }

    return a + d * t;
   

}