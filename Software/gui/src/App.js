import React, { Component } from "react";
import logo from "./logo.svg";
import "./App.css";
import DialControl from "./components/DialControl";

class App extends Component {
	constructor(props) {
		super(props);
		this.state = {
			devMonitors: "",
			settings: {
				"temperature": [
					{value: 1, label: "1", position:1},
					{value: 2, label: "2", position:2},
					{value: 3, label: "3", position:3},
				],
				"humidity": [
					{value: 1, label: "1", position:0},
					{value: 2, label: "2", position:1}, 	
					{value: 3, label: "3", position:2},
					{value: 4, label: "4", position:3},
					{value: 5, label: "5", position:4},
				],
				"lights": [
					{value: 1, label: "1", position:1},
					{value: 2, label: "2", position:2},
					{value: 3, label: "3", position:3},
				],
			},
			curTime: new Date(),
		}
	
		this.pAngle = 0.012;
		this.width = 300;
		this.height = 300;

		this.getItems = this.getItems.bind(this);
		this.sendSettings = this.sendSettings.bind(this);
	}

	componentDidMount(){
		this.timer = setInterval(()=> this.getItems(), 7000);
		setInterval( () => {
			let t = new Date();
			this.setState({
			  curTime : new Date(),
			  arrowAngle : (((t.getHours()*60+t.getMinutes())/1440)*360*Math.PI/180)-Math.PI/2,
			})
		  },6000); // Get time every minute 
	}
	getItems() {
		fetch('/getMonitors')
		.then(res => res.json())
		.then(devMonitors => this.setState({devMonitors}))
		.catch(error => console.log("BAD2", error));
	}
	
	componentWillUnmount() {
		this.timer = null;
	}

	sendSettings() {
		var data= JSON.stringify( this.state.settings );
		console.log(data);
		fetch('/setSettings', {
			method: "POST",
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: data
		})
		.then(function(res){ return res.json(); })
		.then(function(data){ alert( JSON.stringify( data ) ) })
		
	}

	render() {
		return (
			<div className="App">
				<header className="App-header">
					<h1 className="App-title">Welcome to SpeedSeed3</h1>
				</header>
				<p className="App-intro"></p>
				<div >
					<div className="row">
						<div className="col-sm-4">
							<svg height={this.height} width={this.width}>
								<DialControl arrowAngle={this.state.arrowAngle} unit="C" currentValue={this.state.devMonitors.temperature} x={100} y={100} outerRadius={100} innerRadius={50} padAngle={this.pAngle} 
									data={this.state.settings.temperature}
								/>
							</svg>
						</div>
						<div className="col-sm-4">
							<svg height={this.height} width={this.width}>
								<DialControl arrowAngle={this.state.arrowAngle} unit="lum" currentValue={this.state.devMonitors.luminance} x={100} y={100} outerRadius={100} innerRadius={50} padAngle={this.pAngle}
									data={this.state.settings.lights}
								/>
							</svg>
						</div>
						<div className="col-sm-4">
							<svg height={this.height} width={this.width}>
								<DialControl arrowAngle={this.state.arrowAngle} unit="%" currentValue={this.state.devMonitors.humidity} x={100} y={100} outerRadius={100} innerRadius={50} padAngle={this.pAngle}
									data={this.state.settings.humidity}
								/>
							</svg>
						</div>
					</div>
				</div>
			</div>
		);
	}
}

export default App;
