(function() {
	window.Main = {};
	Main.Page = (function() {
		var mosq = null;
		function Page() {
			var _this = this;
			mosq = new Mosquitto();
			
			// VARIÁVEIS GLOBAIS PARA CONTROLE DO PUBLISH DAS VAGAS
			var v1 = '1L';
			var v2 = '2L';
			var v3 = '3L';
			var numVagas = 3;

			// CONECTAR AO BROKER
			$('#connect-button').click(function() {
				return _this.connect();
			});

			// DESCONECTAR DO BROKER
			$('#disconnect-button').click(function() {
				return _this.disconnect();
			});

			// ABRE ENTRADA
			$('#abre-entrada').click(function() {
				var payload = "EO";  
				var TopicPublish = $('#pub-topic-text')[0].value;				
				mosq.publish(TopicPublish, payload, 0);
			});

			// FECHA ENTRADA
			$('#fecha-entrada').click(function() {
				var payload = "EC";  
				var TopicPublish = $('#pub-topic-text')[0].value;		
				mosq.publish(TopicPublish, payload, 0);
			});

			// ABRE SAIDA
			$('#abre-saida').click(function() {
				var payload = "SO";  
				var TopicPublish = $('#pub-topic-text')[0].value;				
				mosq.publish(TopicPublish, payload, 0);
			});

			// FECHA SAIDA
			$('#fecha-saida').click(function() {
				var payload = "SC";  
				var TopicPublish = $('#pub-topic-text')[0].value;				
				mosq.publish(TopicPublish, payload, 0);
			});

			// QUANDO CONECTADO AO BROKER
			mosq.onconnect = function(rc){
				var debug = document.createElement("div");
				var topic = $('#pub-subscribe-text')[0].value;
				debug.className += "alert alert-success";
				debug.innerHTML = "Conectado ao Broker em "+ _this.dateFormat();
				$("#debug").append(debug);
				mosq.subscribe(topic, 0);
			};

			// QUANDO A CONEXÃO É DESFEITA
			mosq.ondisconnect = function(rc){
				var url = "ws://iot.eclipse.org:80/ws";		
				var debug = document.createElement("div");
				debug.className += "alert alert-danger";
				debug.innerHTML = "A conexão com o broker foi perdida em "+ _this.dateFormat();
				$("#debug").append(debug);	
			};

			// A CADA EVENTO
			mosq.onmessage = function(topic, payload, qos){	
				var acao = payload[0] + payload[1];
				var TopicPublish = $('#pub-topic-text')[0].value;	
				
				console.log(acao);
				
				// ENTRADA ABERTA
				if (acao == 'EO'){
					document.getElementById("status_entrada").className = "panel panel-green";
					document.getElementById("entrada_sts").innerHTML = "Entrada Aberta";
					document.getElementById("entrada_icon").className = "fa fa-check fa-3x";
				}

				// ENTRADA FECHADA
				if(acao == 'EC'){
					document.getElementById("status_entrada").className = "panel panel-red";
					document.getElementById("entrada_sts").innerHTML = "Entrada Fechada";
					document.getElementById("entrada_icon").className = "fa fa-times fa-3x";
				}
								
				// SAIDA ABERTA
				if(acao == 'SO'){
					document.getElementById("status_saida").className = "panel panel-green";
					document.getElementById("saida_sts").innerHTML = "Saída Aberta";
					document.getElementById("saida_icon").className = "fa fa-check fa-3x";
				}
				
				// SAIDA FECHADA
				if(acao == 'SC'){
					document.getElementById("status_saida").className = "panel panel-red";
					document.getElementById("saida_sts").innerHTML = "Saída Fechada";
					document.getElementById("saida_icon").className = "fa fa-times fa-3x";
				}
				
				// Vaga 1 ocupada
				if(acao == '1O' && v1 == '1L'){
					v1 = '1O';
					mosq.publish(TopicPublish, acao, 0);
					document.getElementById("status_vaga_1").className = "panel panel-red";
					document.getElementById("desc_vaga_1").innerHTML = "Vaga 1 Ocupada";
					numVagas--;
				}
				
				// Vaga 1 livre
				if(acao == '1L' && v1 == '1O'){
					v1 = '1L';
					mosq.publish(TopicPublish, acao, 0);
					document.getElementById("status_vaga_1").className = "panel panel-green";
					document.getElementById("desc_vaga_1").innerHTML = "Vaga 1 Livre";
					numVagas++;
				}

				// Vaga 2 ocupada
				if(acao == '2O' && v2 == '2L'){
					v2 = '2O';
					mosq.publish(TopicPublish, acao, 0);
					document.getElementById("status_vaga_2").className = "panel panel-red";
					document.getElementById("desc_vaga_2").innerHTML = "Vaga 2 Ocupada";
					numVagas--;
				}
				
				// Vaga 2 livre
				if(acao == '2L' && v2 == '2O'){
					v2 = '2L';
					mosq.publish(TopicPublish, acao, 0);
					document.getElementById("status_vaga_2").className = "panel panel-green";
					document.getElementById("desc_vaga_2").innerHTML = "Vaga 2 Livre";
					numVagas++;
				}

				// Vaga 3 ocupada
				if(acao == '3O' && v3 == '3L'){
					v3 = '3O';
					mosq.publish(TopicPublish, acao, 0);
					document.getElementById("status_vaga_3").className = "panel panel-red";
					document.getElementById("desc_vaga_3").innerHTML = "Vaga 3 Ocupada";
					numVagas--;
				}
				
				// Vaga 3 livre
				if(acao == '3L' && v3 == '3O'){
					v3 = '3L';
					mosq.publish(TopicPublish, acao, 0);
					document.getElementById("status_vaga_3").className = "panel panel-green";
					document.getElementById("desc_vaga_3").innerHTML = "Vaga 3 Livre";
					numVagas++;
				}

				document.getElementById("num_vagas").innerHTML = "0" + numVagas;
			};
		}

		Page.prototype.connect = function(){
			var url = "ws://iot.eclipse.org:80/ws"; // servidor de broker MQTT
			mosq.connect(url);
		};
		
		Page.prototype.disconnect = function(){
			mosq.disconnect();
		};

		Page.prototype.dateFormat = function(){
			var meses = new Array("Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez");
			var date =  new Date();
			var dia = date.getDay() + ' ' + meses[date.getMonth()] + " " + date.getFullYear();  
			var hora = date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();

			return dia + " " + hora;
		}
				
		return Page;
	})();
	$(function(){
		return Main.controller = new Main.Page;
	});
}).call(this);

