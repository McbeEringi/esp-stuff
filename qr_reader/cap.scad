$fa=5;$fs=.2;


difference(){
	union(){
		scale(-1)linear_extrude(1.5)square([13,10],center=true);
		linear_extrude(2)circle(d=7);
	}
	translate([0,0,2-2.5/2])rotate_extrude()difference(){
		square(2.5);
		translate([2.5,0])circle(d=2.5);
	}
	linear_extrude(10,center=true){
		circle(d=2.5);
		translate([-2,0])minkowski(){
			circle(d=1);
			square([1e-9,3],center=true);
		}
	}
}


