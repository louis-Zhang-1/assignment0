var can1; var can2; 
var ctx1; var cts2;

var canWidth; var canHeight;
var lastTime; var ATime; var deltaTime;

var bgPic = new Image();

var ane;  var fruit; var fish;  var data; var shark; var gameTime;
// the loc of the mouse
var mx; var my;
var isRecord = false;

function recordScore(){
    if (typeof(Storage) !== "undefined") {
        localStorage.setItem("bestScore", data.score);
        // Retrieve
      } else {
        console.log( "Sorry, your browser does not support Web Storage...");
      }
}

function recordTime(){
    if (typeof(Storage) !== "undefined") {
        localStorage.setItem("playTime", Math.floor(gameTime / 1000));
        // Retrieve
      } else {
        console.log( "Sorry, your browser does not support Web Storage...");
      }
}

function getScore(){
    if(localStorage.getItem("bestScore") == null){
        return 0;
    }else{
        return localStorage.getItem("bestScore");
    }
      
}

function drawBackground(){
    ctx2.drawImage(bgPic, 0, 0, canWidth, canHeight);
}


function game(){
    init();
    lastTime = Date.now();
    gameLoop();
}

function init(){
    can1 = document.getElementById("canvas1");  // fishes dust circle
    ctx1 = can1.getContext('2d');
    can2 = document.getElementById("canvas2"); // background fruits
    ctx2 = can2.getContext('2d');
    gameTime = 0;
    ATime = 0;

    can1.addEventListener('mousemove', onMouseMove, false);
    canWidth = can1.width;
    canHeight = can2.height;
    bgPic.src = "./src/background.jpg";
    shark = new sharkObj();
    shark.init();
    ane = new aneObj();
    ane.init();
    fruit = new fruitObj();
    fruit.init();
    fish = new fishObj();
    fish.init();
    data = new dataObj();
    mx = canWidth * 0.5;
    my = canHeight * 0.5;


    ctx1.font = "20px Verdana";
    ctx1.textAlign = "left";
}

function gameLoop(){
    if(data.score > getScore()){
        recordScore();
    }
    recordTime();

    window.requestAnimationFrame(gameLoop);
    var now = Date.now();
    deltaTime = now - lastTime;
    lastTime = now;
    if(!data.gameOver){
        gameTime += deltaTime;
    }
    if(deltaTime > 40) deltaTime = 40;
    drawBackground();
    ane.draw();
    fruitMoniter();
    fruit.draw();
    ctx1.clearRect(0, 0, canWidth, canHeight);
    fish.draw();
    shark.draw();
    fishFruitCollision();
    fishSharkCollision();
    data.draw();
}


function onMouseMove(e){
    if(!data.gameOver){
        if(e.offSetX || e.layerX){
            mx = e.offSetX == undefined ? e.layerX : e.offSetX;
            my = e.offSetX == undefined ? e.layerY : e.offSetY;
           
        }
    }
}

var aneObj = function(){
    this.x = [];
    this.len = [];
 }

 aneObj.prototype.num = 50;
 aneObj.prototype.init = function(){
     for(var i = 0; i < this.num; i++){
         this.x[i] = i * 16 + Math.random() * 20;
         this.len[i] = 100 + Math.random() * 50;
     }
}

aneObj.prototype.draw = function(){
    ctx2.save();
    ctx2.globalAlpha = 0.6;
    ctx2.strokeStyle = "yellow";
    ctx2.lineWidth = 15;
    ctx2.lineCap = "round";
    for(var i = 0; i < this.num; i++){
       ctx2.beginPath();
       ctx2.moveTo(this.x[i], canHeight);
       ctx2.lineTo(this.x[i], canHeight - this.len[i]);
       ctx2.stroke();
    }
    ctx2.restore();
}

function dis(x1, y1, x2, y2) {
	return Math.pow(x1 - x2, 2) + Math.pow(y1 - y2, 2);
}
function fishFruitCollision(){
    if(!data.gameOver){
        for(let i = 0; i < fruit.num; i++){
            if(fruit.alive[i]){
                let l = dis(fruit.x[i], fruit.y[i], fish.x, fish.y);
                if(l < 800){
                    fruit.dead(i);
                    data.fruitNum++;
                    if(fruit.fruitType[i] == "blue"){
                        if(fish.health < 20){
                            fish.health -= 6;
                        }else if(fish.health < 50){
                            fish.health -= 4;
                        }else if(fish.health < 70){
                            fish.health -= 2;
                        }else if(fish.health < 100){
                            fish.health -= 1;
                        }
                        if(data.score > 0){
                            data.score  -= 1;  
                        }
                       
                    }else{
                        if(fish.health < 10){
                            fish.health += 8;
                        }else if(fish.health < 30){
                            fish.health += 6;
                        }else if(fish.health < 50){
                            fish.health += 4;
                        }else if(fish.health < 70){
                            fish.health += 2;
                        }else if(fish.health < 100){
                            fish.health += 1;
                        }
                        data.score  += 1;  
                    }
                }
            }
        }
    }   
} 

function fishSharkCollision(){
    for(var i = 0; i < shark.num; i++){
        var l = dis(fish.x, fish.y, shark.x[i], shark.y[i]);
        if(!data.gameOver){
            if(l < 1000){
               data.gameOver = true;
            }
        }
    }
}


window.requestAnimFrame = (function() {
    return window.requestAnimationFrame 
           || function( /* function FrameRequestCallback */ callback, /* DOMElement Element */ element) {
			return window.setTimeout(callback, 3000 / 60);
		};
})();

var dataObj = function(){
    this.double = 1;
    this.score = 0;
    this.gameOver = false;
}

dataObj.prototype.reset = function(){
    this.fruitNum = 0;
    this.double = 1;
}

dataObj.prototype.draw = function(){
    var w = can1.width;
    var h = can1.height;


    ctx1.fillStyle = "black";
    ctx1.fillText("Health: " + Math.round(fish.health), w * 0.8, 50);
    ctx1.fillText("Score: " + this.score, w * 0.8, 80);
    ctx1.fillText("Time: " + Math.floor(gameTime / 1000) +"s", w * 0.8, 110);
    ctx1.fillText("Best Score: " + getScore(), w * 0.8, 140);
    

    if(this.gameOver){

        ctx1.fillText("GAME OVER", w * 0.5, h * 0.5);
    }
}

dataObj.prototype.addScore = function(){
    this.score += this.fruitNum * 100 * this.double;
    this.fruitNum = 0;
    this.double = 1;
}

