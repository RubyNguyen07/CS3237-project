import React, { useEffect, useState } from 'react';
import { format } from "date-fns";
import chickenImg from "./images/chickenrice.jpg"
import fishImg from "./images/fishsoup.jpg"
import saladImg from "./images/salad.png"
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import CardActions from '@mui/material/CardActions';
import CardMedia from '@mui/material/CardMedia';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Pagination from '@mui/material/Pagination';
import { Dialog, DialogContent, DialogTitle, } from '@mui/material';

function App() {
    const [orders, setOrders] = useState([]);
    const [open, setOpen] = React.useState(false);
    const [predictedOrder, setPredictedOrder] = useState({});
    const [page, setPage] = useState(1);
    const ordersPerPage = 10;
    const totalPages = Math.ceil(orders.length / ordersPerPage);

    const handlePageChange = (event, value) => {
        setPage(value);
    };
    const startIndex = (page - 1) * ordersPerPage;
    const currentOrders = orders.slice(startIndex, startIndex + ordersPerPage);

    const handleOpen = (startOfWeek) => {
        fetch(`http://localhost:5001/api/predict?date=${startOfWeek}`)
            .then(response => response.json())
            .then(data => setPredictedOrder(data))
            .catch(error => console.error('Error fetching predicted order:', error));
        setOpen(true);
    };
    const handleClose = () => {
        setPredictedOrder({});
        setOpen(false);
    };

    const getImageUrl = (chickenOrder, fishOrder, saladOrder) => {
        if (chickenOrder >= Math.max(fishOrder, saladOrder)) {
            return chickenImg;
        } else if (fishOrder >= Math.max(chickenOrder, saladOrder)) {
            return fishImg;
        }
        return saladImg;
    }

    useEffect(() => {
        // Fetch items from Flask backend
        fetch('http://localhost:5001/api/order')
            .then(response => response.json())
            .then(data => setOrders(data["orders"]))
            .catch(error => console.error('Error fetching items:', error));
    }, []);

    return (
        <div className="App">
            <h1 style={{ justifySelf: 'center', color: "#fc7100" }}>Order List</h1>
            <ul className="horizontal-list" style={{ justifySelf: 'center', maxWidth: '100%' }}>
                {currentOrders.map(order => (
                    //   <li key={order[0]}>{format(new Date((order[1])), "dd-MM-yyyy") + " | " + order[2] + "," + order[3] + "," + order[4]}</li>
                    <li key={order[0]}>
                        <Card sx={{ display: 'flex', maxWidth: '100%', backgroundColor: '#fb8818' }}>
                            <CardMedia
                                component="img"
                                height="100%"
                                image={getImageUrl(order[2], order[3], order[4])}
                                alt="Most ordered dish"
                                sx={{ maxWidth: 200 }}
                            />
                            <CardContent sx={{ flex: '1' }}>
                                <Typography variant="h6" sx={{ color: '#ff007f', fontWeight: 600 }}>
                                    Week of {format(new Date((order[1])), "dd-MM-yyyy")}
                                </Typography>
                                <Typography variant="body3" sx={{ color: '#FDE4CE' }}>
                                    Chicken rice orders: {order[2]} <br />
                                    Fish soup orders: {order[3]} <br />
                                    Salad orders: {order[4]} <br />
                                </Typography>
                            </CardContent>
                            <CardActions disableSpacing sx={{ flex: '1' }}>
                                <Button onClick={() => handleOpen(format(new Date((order[1])), "yyyy-MM-dd"))} sx={{ backgroundColor: '#5ca135', color: '#FDE4CE' }}>Predict the week's order</Button>

                                <Dialog
                                    open={open}
                                    onClose={handleClose}
                                    BackdropProps={{
                                        style: {
                                            backgroundColor: 'rgba(0.2, 0.2, 0.2, 0.05)', // Semi-transparent white backdrop
                                        },
                                    }}
                                    PaperProps={{
                                        style: {
                                            backgroundColor: '#5ca135', // White background for the dialog content
                                            opacity: 1, // Ensure dialog content is fully opaque
                                            width: '500px',  // Custom width
                                            height: '300px', // Custom height
                                            alignItems: 'center'
                                        },
                                    }}
                                >
                                    <DialogTitle>
                                        <Typography variant="h5" component="h2" sx={{ color: '#FDE4CE', fontWeight: 600 }}>
                                            Predicted order:
                                        </Typography>
                                    </DialogTitle>
                                    <DialogContent>

                                        <Typography sx={{ mt: 2, color: '#FDE4CE' }}>
                                            Predicted chicken rice orders: {predictedOrder["predictedChickenOrder"]} <br />
                                            Predicted fish soup orders: {predictedOrder["predictedFishOrder"]} <br />
                                            Predicted salad orders: {predictedOrder["predictedSaladOrder"]} <br />
                                        </Typography>
                                    </DialogContent>

                                    {/* </Box> */}
                                </Dialog>
                            </CardActions>
                        </Card>
                    </li>
                ))}
            </ul>
            <Pagination
                count={totalPages}
                page={page}
                onChange={handlePageChange}
                color="primary"
                size="medium"
                sx={{
                    justifySelf: 'center',
                    '& .MuiPaginationItem-root': {
                        color: '#5CA135', // Default color for pagination items
                    },
                    '& .Mui-selected': {
                        backgroundColor: '#5CA135', // Selected page background color
                        color: '#FDE4CE', // Selected page text color
                    },
                    marginBottom: 5
                }}
            />
        </div>
    );
}

export default App;