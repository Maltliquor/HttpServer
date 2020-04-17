var table = [
	"信号量的使用", "2020.03.29",
	"He", "Helium", 
	"Li", "Lithium", 
	"Be", "Beryllium", 
	"B", "Boron", 
	"C", "Carbon",
	"N", "Nitrogen",
	"O", "Oxygen", 
	"F", "Fluorine", 
	"Ne", "Neon",
	"Na", "Sodium",
	
];

var camera, scene, renderer;
var controls;

var objects = [];
var targets = { table: [], sphere: [], helix: [], grid: [] };

init();
animate();

function init() {

	camera = new THREE.PerspectiveCamera( 40, window.innerWidth / window.innerHeight, 1, 10000 );
	camera.position.z = 3000;

	scene = new THREE.Scene();

	// table
	var ele_width =  window.innerWidth/3  ;
	var ele_height = window.innerHeight/2 ;
	var step = 2;
	for ( var i = 0; i < table.length; i += step ) {

		var element = document.createElement( 'div' );
		element.className = 'element';
		element.style.backgroundColor = 'rgba(0,127,127,' + ( Math.random() * 0.5 + 0.25 ) + ')';
		element.style.height = ele_height.toString() + "px";  
		element.style.width = ele_width.toString() + "px"; 
		
		var date = document.createElement( 'div' );
		date.className = 'date';
		date.textContent = table[ i+1 ];
		element.appendChild( date );

		var symbol = document.createElement( 'div' );
		symbol.className = 'symbol';
		symbol.textContent = table[ i ];
		element.appendChild( symbol );

		//var details = document.createElement( 'div' );
		//details.className = 'details';
		//details.innerHTML = table[ i + 1 ] + '<br>' + table[ i + 2 ];
		//element.appendChild( details );

		var object = new THREE.CSS3DObject( element );
		object.position.x = Math.random() * 4000 - 2000;
		object.position.y = Math.random() * 4000 - 2000;
		object.position.z = Math.random() * 4000 - 2000;
		scene.add( object );

		objects.push( object );

		
		var object = new THREE.Object3D();
		object.position.x = ( (parseInt(i/step)%5+1) * (ele_width+ 20) ) - window.innerWidth; //5表示一行放5个元素
		object.position.y = window.innerHeight - ( (parseInt(parseInt(i/step)/5) +1) * (ele_height+ 60) ) ;

		targets.table.push( object );

	}

	// sphere

	var vector = new THREE.Vector3();
	var sphere_radius = 800;
	for ( var i = 0, l = objects.length; i < l; i ++ ) {

		var phi = Math.acos( -1 + ( 2 * i ) / l );
		var theta = Math.sqrt( l * Math.PI ) * phi;

		var object = new THREE.Object3D();

		object.position.x = sphere_radius * Math.cos( theta ) * Math.sin( phi );
		object.position.y = sphere_radius * Math.sin( theta ) * Math.sin( phi );
		object.position.z = sphere_radius * Math.cos( phi );

		vector.copy( object.position ).multiplyScalar( 2 );

		object.lookAt( vector );

		targets.sphere.push( object );

	}

	// helix

	var vector = new THREE.Vector3();
	
	var helix_radius = 900;
	var helix_step = ele_height/9;
	var angle = Math.asin(ele_width/2/helix_radius) * 2 //0.175;
	for ( var i = 0, l = objects.length; i < l; i ++ ) {

		var phi = i * angle + Math.PI;

		var object = new THREE.Object3D();

		object.position.x = helix_radius * Math.sin( phi );
		object.position.y = - ( i * helix_step ) + helix_radius/2;
		object.position.z = helix_radius * Math.cos( phi );

		vector.x = object.position.x * 2;
		vector.y = object.position.y;
		vector.z = object.position.z * 2;

		object.lookAt( vector );

		targets.helix.push( object );

	}

	// grid
	var grid_radius = ele_width+60;
	for ( var i = 0; i < objects.length; i ++ ) {

		var object = new THREE.Object3D();

		object.position.x = ( ( i % 3 ) * grid_radius ) - grid_radius;
		object.position.y = ( - ( Math.floor( i / 3 ) % 3 ) * grid_radius ) + grid_radius;
		object.position.z = ( Math.floor( i / 9 ) ) * grid_radius*2.5 - grid_radius*3;

		targets.grid.push( object );

	}

	//

	renderer = new THREE.CSS3DRenderer();
	renderer.setSize( window.innerWidth, window.innerHeight );
	renderer.domElement.style.position = 'absolute';
	document.getElementById( 'container' ).appendChild( renderer.domElement );

	//

	controls = new THREE.TrackballControls( camera, renderer.domElement );
	controls.rotateSpeed = 0.5;
	controls.minDistance = 500;
	controls.maxDistance = 6000;
	controls.addEventListener( 'change', render );

	var button = document.getElementById( 'table' );
	button.addEventListener( 'click', function ( event ) {

		transform( targets.table, 2000 );

	}, false );

	var button = document.getElementById( 'sphere' );
	button.addEventListener( 'click', function ( event ) {

		transform( targets.sphere, 2000 );

	}, false );

	var button = document.getElementById( 'helix' );
	button.addEventListener( 'click', function ( event ) {

		transform( targets.helix, 2000 );

	}, false );

	var button = document.getElementById( 'grid' );
	button.addEventListener( 'click', function ( event ) {

		transform( targets.grid, 2000 );

	}, false );

	transform( targets.table, 5000 );

	//

	window.addEventListener( 'resize', onWindowResize, false );

}

function transform( targets, duration ) {

	TWEEN.removeAll();

	for ( var i = 0; i < objects.length; i ++ ) {

		var object = objects[ i ];
		var target = targets[ i ];

		new TWEEN.Tween( object.position )
			.to( { x: target.position.x, y: target.position.y, z: target.position.z }, Math.random() * duration + duration )
			.easing( TWEEN.Easing.Exponential.InOut )
			.start();

		new TWEEN.Tween( object.rotation )
			.to( { x: target.rotation.x, y: target.rotation.y, z: target.rotation.z }, Math.random() * duration + duration )
			.easing( TWEEN.Easing.Exponential.InOut )
			.start();

	}

	new TWEEN.Tween( this )
		.to( {}, duration * 2 )
		.onUpdate( render )
		.start();

}

function onWindowResize() {

	camera.aspect = window.innerWidth / window.innerHeight;
	camera.updateProjectionMatrix();

	renderer.setSize( window.innerWidth, window.innerHeight );

	render();

}

function animate() {

	requestAnimationFrame( animate );

	TWEEN.update();

	controls.update();

}

function render() {

	renderer.render( scene, camera );

}