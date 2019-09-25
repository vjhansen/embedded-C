unsigned char ledcode(unsigned char digit){
	if (digit==0)
		return 0b11101110;
	else if(digit==1)
		return 0b00100100;
	else if(digit==2)
		return 0b10111010;
	else if(digit==3)
		return 0b10110110;
	else if(digit==4)
		return 0b01110100;
	else if(digit==5)
		return 0b11010110;
	else if(digit==6)
		return 0b11011110;
	else if(digit==7)
		return 0b10100100;
	else if(digit==8)
		return 0b11111110;
	else if(digit==9)
		return 0b11110100;
	else
		return 0;
}
