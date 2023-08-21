
# Sistema de Registro de Asistencia por RFID

Un sistema de Registro de Asistencia mediante un controlador ESP32. Los datos pueden ser visualizados mediante una Web cuyo código fuente está en el siguiente repositorio [RFID Attendance App](https://github.com/dchilo/rfid-attendance-app), además todos los datos son almacenados en una base de datos en MongoDB y consultados por API, el código del backend se encuentra en [RFID Attendance Api](https://github.com/dchilo/rfid-attendance-api).



## API Reference / Rutas API

#### Vincular tarjeta RFID con un usuario / Link user and RFID card

```http
  POST /addcode?${id}%${token}
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `token` | `string` | **Required**. Your API token |
| `user_id` | `string` | **Required**. User ID to link RFID code|

#### Registrar asistencia de una asignatura en especifico

```http
  POST /attendance?${courseId}%{isTaking}
```

| Parameter | Type     | Description                       |
| :-------- | :------- | :-------------------------------- |
| `courseId`      | `string` | **Required**. Id to create the attendance |
| `isTaking`      | `string` | **Required**. |



## Diagram / Implementación

Diagrama de conexiones para el ESP32 / Diagram

![App Screenshot](https://github.com/dchilo/rfid-attendance-esp32/blob/main/Diagrama.jpg?raw=true)

## Deploy

Para configurar correctamente todo configurar la url de la API del backend en el código.

## Authors

- [@dchilo](https://www.github.com/dchilo)


## 🔗 Links

[![linkedin](https://img.shields.io/badge/linkedin-0A66C2?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/davidchilo/)

