import React, { Component } from "react";
import TimePicker from 'react-times';
import logo from "./logo.svg";
import "./App.css";
import DialControl from "./components/DialControl";
// use material theme
import 'react-times/css/material/default.css';
// or you can use classic theme
//import 'react-times/css/classic/default.css';

class App extends Component {
	state = {
		devMonitors: [],
		tempSettings: [],
		humSettings: [],
		lumSettings: [],
	}
	constructor(props) {
		super(props);
		this.pAngle = 0.012;
		this.width = 300;
		this.height = 300;
	}

	componentDidMount(){
		this.timer = setInterval(()=> this.getItems(), 5000);
	}
	getItems() {
		fetch('/getMonitors')
		.then(res => res.json())
		.then(devMonitors => this.setState({devMonitors}));
	  }
	
	componentWillUnmount() {
		this.timer = null;
	  }
	
	render() {
		return (
			<div className="App">
				<header className="App-header">
					<h1 className="App-title">Welcome to SpeedSeed3</h1>
				</header>
				<p className="App-intro"></p>
				<div >
					<div class="row">
						<div class="col-sm-4">
							<svg height={this.height} width={this.width}>
								<DialControl currentValue="Temperature" x={100} y={100} outerRadius={100} innerRadius={50} padAngle={this.pAngle} 
									data={
										[
											{value: 10, label: "-2 C"},
											{value: 30, label: "16 C"},
											{value: 80, label: "30 C"}
										]
									}
								/>
							</svg>
						</div>
						<div class="col-sm-4">
							<svg height={this.height} width={this.width}>
								<DialControl currentValue="Illumination" x={100} y={100} outerRadius={100} innerRadius={50} padAngle={this.pAngle}
									data={
										[
											{value: 30, label: "5 lum"},
											{value: 90, label: "100 lum"},
											{value: 100, label: "20 lum"}
										]
									}
								/>
							</svg>
						</div>
						<div class="col-sm-4">
							<svg height={this.height} width={this.width}>
								<DialControl currentValue="Humidity" x={100} y={100} outerRadius={100} innerRadius={50} padAngle={this.pAngle}
									data={
										[
											{value: 20, label: "20 %"},
											{value: 30, label: "25 %"},
											{value: 45, label: "45 %"},
											{value: 50, label: "60 %"},
											{value: 80, label: "40 %"}
										]
									}
								/>
							</svg>
						</div>
					</div>
				</div>
				<TimePicker colorPalette="dark"/>
				<div class="float-left">
					<form padding-top="20">
						<div class="form-group row">
							<label class="col-sm-2 col-form-label" for="fromInput">From:</label>
							<div class="col-sm-10">
							</div>
						</div>
						<div class="form-group row">
							<label class="col-sm-2 col-form-label"  for="toInput">To:</label>
							<div class="col-sm-10">
								<input id="toInput" class="form-control" type="text"></input>
							</div>
						</div>
						<div class="form-group row">
							<label class="col-sm-2 col-form-label" for="valueInput">Value:</label>
							<div class="col-sm-10">
								<input id="valueInput" class="form-control" type="text"></input>
							</div>
						</div>
						<button class="btn btn-primary">Submit</button>
					</form>
				</div>
				<h1>Data from /getMonitors</h1>
				<div>
					<ul>
						<li>{this.state.devMonitors.temperature}</li>
						<li>{this.state.devMonitors.humidity}</li>
						<li>{this.state.devMonitors.luminance}</li>
					</ul>
				</div>
			</div>
		);
	}
}

export default App;