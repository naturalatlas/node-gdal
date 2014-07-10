//js implementation of OGREnvelope class

var Envelope = module.exports = function(envelope){
	if (envelope) {
		this.minX = envelope.minX;
		this.minY = envelope.minY;
		this.maxX = envelope.maxX;
		this.maxY = envelope.maxY;
	} else {
		this.minX = 0;
		this.minY = 0;
		this.maxX = 0;
		this.maxY = 0;
	}
};

Envelope.prototype.isEmpty = function(){ 
	return this.minX != 0 && this.minY != 0 && this.maxX != 0 && this.maxY != 0;
};

Envelope.prototype.merge = function(envelope){
 	if (arguments.length == 1) {
		if (this.isEmpty()) {
			this.minX = envelope.minX;
			this.maxX = envelope.maxX;
			this.minY = envelope.minY;
			this.maxY = envelope.maxY;
		} else {
			this.minX = Math.min(envelope.minX, this.minX);
			this.maxX = Math.max(envelope.maxX, this.maxX);
			this.minY = Math.min(envelope.minY, this.minY);
			this.maxY = Math.max(envelope.maxY, this.maxY);
		}
	} else {
		var x = arguments[0];
		var y = arguments[1];
		if (this.isEmpty()) {
			this.minX = this.maxX = x;
			this.minY = this.maxY = y;
		} else {
			this.minX = Math.min(x, this.minX);
			this.maxX = Math.max(x, this.maxX);
			this.minY = Math.min(y, this.minY);
			this.maxY = Math.max(y, this.maxY);
		}
	}
};

Envelope.prototype.intersects = function(envelope) {
	return this.minX <= envelope.maxX && this.maxX >= envelope.minX && 
	       this.minY <= envelope.maxY && this.maxY >= envelope.minY;
};

Envelope.prototype.intersect = function(envelope){
	if (this.intersects(envelope)) {
		if (this.isEmpty()) {
			this.minX = envelope.minX;
			this.maxX = envelope.maxX;
			this.minY = envelope.minY;
			this.maxY = envelope.maxY;
		} else {
			this.minX = Math.max(envelope.minX, this.minX);
			this.maxX = Math.min(envelope.maxX, this.maxX);
			this.minY = Math.max(envelope.minY, this.minY);
			this.maxY = Math.min(envelope.maxY, this.maxY);
		}
	} else {
		this.minX = this.maxX = this.minY = this.maxY = 0;
	}
};

Envelope.prototype.contains = function(envelope){
	return this.minX <= envelope.minX && this.minY <= envelope.minY && 
	       this.maxX >= envelope.maxX && this.maxY >= envelope.maxY;
};