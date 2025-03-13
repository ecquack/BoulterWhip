async function refresh_clock() {
	the_now=new Date();
	the_now=the_now.toString();
	all_now=the_now.split('G');
	document.getElementById("time_display").innerHTML = all_now[0];//.toString();

	let scandata=await sendCommand("refresh");
	ShowScan(scandata);

}

		
async function sendCommand(full_url)
{
	try {
		let res = await fetch(full_url);
		return await res.text();
	} catch (error) {
		console.log(error);
	}

}

function ShowScan(scandata) {
//	console.log("Ended scan");
	preid=document.getElementById("scan_result");
	preid.innerHTML=scandata;
//	console.log("Displayed scan");
	
	passfail=document.getElementById("passfail");
	if(scandata[0]=="G") {
		passfail.innerHTML="FAIL";
		passfail.setAttribute("style","color:red;");
	}
	else if(scandata[0]=="\r") 
		{
		passfail.innerHTML="PASS";
		passfail.setAttribute("style","color:green;");
	}
	else passfail.innterHTML=""; // no result yet- fresh reboot
	// weird case. what's the first character of a pass? carriage return...
}

async function DoSave()
{
	console.log("Saving results");

	technician=document.getElementById("technician").value
	serialnumber=document.getElementById("serialnumber").value
	passfail=document.getElementById("passfail").innerHTML
	datetime=document.getElementById("time_display").innerHTML;
    results=document.getElementById("scan_result").innerHTML;
	console.log("Saving:")
	console.log(technician);
	console.log(serialnumber);
	console.log(passfail);
	console.log(datetime);
	console.log(results);

//  main.js
 
	const formData = new FormData();
	formData.append("technician", technician);
	formData.append("serialnumber", serialnumber);
	formData.append("datetime", datetime);
	formData.append("passfail", passfail);
	formData.append("results", results);

// POST request using fetch()
fetch("http://192.168.1.100:8086/savescan", {
	mode:  'no-cors' ,
    
    // Adding method type
    method: "POST",
    
	body:
		formData
	});

}


async function DoScan()
{
	console.log("Starting scan");
	let scandata=await sendCommand("scanreport");
	ShowScan(scandata);
	
}

window.addEventListener('load', function () {
	var fetchInterval = 1000; // 1 second.
	setInterval(refresh_clock, fetchInterval);
	});

